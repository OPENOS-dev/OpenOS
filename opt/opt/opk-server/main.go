// OPK Repository Server
//
// Serves OPK package repositories over HTTP.
// This is the server that opk CLI and opk-ui communicate with.
//
// Endpoints:
//   GET  /Packages.json                     Repository index
//   GET  /pool/{first_char}/{filename}       Package file download
//   GET  /source/{first_char}/{filename}     Source tarball download
//   GET  /stats                              Repository statistics
//   POST /api/upload                         Upload new package (auth required)
//
// Usage:
//   go run . --repo=/var/opk/repo --addr=:8080

package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/gorilla/mux"
	"github.com/rs/cors"
)

// ============================================================
// Data Types
// ============================================================

type PackageManifest struct {
	Name          string   `json:"name"`
	Version       string   `json:"version"`
	Description   string   `json:"description"`
	Section       string   `json:"section"`
	Maintainer    string   `json:"maintainer"`
	Homepage      string   `json:"homepage"`
	License       string   `json:"license"`
	Architecture  string   `json:"architecture"`
	PackageSize   uint64   `json:"package_size"`
	InstalledSize uint64   `json:"installed_size"`
	Filename      string   `json:"filename"`
	SHA256        string   `json:"sha256"`
	Depends       []string `json:"depends"`
	Provides      []string `json:"provides"`
	Conflicts     []string `json:"conflicts"`
	Priority      string   `json:"priority"`
	Tags          []string `json:"tags"`
	Origin        string   `json:"origin"`
}

type RepositoryIndex struct {
	Origin        string             `json:"origin"`
	Label         string             `json:"label"`
	Description   string             `json:"description"`
	Components    []string           `json:"components"`
	Architectures []string           `json:"architectures"`
	Packages      []PackageManifest  `json:"packages"`
	IndexSHA256   string             `json:"index_sha256"`
	GeneratedAt   string             `json:"generated_at"`
}

type Stats struct {
	TotalPackages    int    `json:"total_packages"`
	TotalArchives    int    `json:"total_archives"`
	TotalSizeBytes   uint64 `json:"total_size_bytes"`
	LastUpdated      string `json:"last_updated"`
	RepositoryOrigin string `json:"repository_origin"`
}

// ============================================================
// Server
// ============================================================

type Server struct {
	repoDir    string
	index      *RepositoryIndex
	packages   map[string]PackageManifest
	indexPath  string
}

func NewServer(repoDir string) *Server {
	return &Server{
		repoDir:   repoDir,
		packages:  make(map[string]PackageManifest),
		indexPath: filepath.Join(repoDir, "Packages.json"),
	}
}

// Scan repository directory for .opk packages and build the index
func (s *Server) BuildIndex() error {
	log.Printf("Building package index from: %s", s.repoDir)

	s.index = &RepositoryIndex{
		Origin:        "Open-OS Repository",
		Label:         "Open-OS Stable",
		Description:   "Official Open-OS package repository",
		Components:    []string{"main", "universe"},
		Architectures: []string{"amd64", "arm64"},
		Packages:      make([]PackageManifest, 0),
		GeneratedAt:   time.Now().UTC().Format(time.RFC3339),
	}

	// Walk the pool directory
	err := filepath.Walk(s.repoDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if info.IsDir() || filepath.Ext(path) != ".opk" {
			return nil
		}

		// Try to read manifest from the .opk file alongside
		manifestPath := strings.TrimSuffix(path, ".opk") + ".json"
		if _, err := os.Stat(manifestPath); os.IsNotExist(err) {
			return nil // No manifest, skip
		}

		data, err := os.ReadFile(manifestPath)
		if err != nil {
			return fmt.Errorf("reading manifest %s: %w", manifestPath, err)
		}

		var pkg PackageManifest
		if err := json.Unmarshal(data, &pkg); err != nil {
			return fmt.Errorf("parsing manifest %s: %w", manifestPath, err)
		}

		// Compute SHA256 of the .opk file
		hash, size, err := computeSHA256(path)
		if err != nil {
			return fmt.Errorf("hashing %s: %w", path, err)
		}

		pkg.PackageSize = size
		pkg.SHA256 = hash
		pkg.Filename = filepath.Base(path)
		pkg.Origin = "openos"

		s.packages[pkg.Name] = pkg
		s.index.Packages = append(s.index.Packages, pkg)

		return nil
	})

	if err != nil {
		return fmt.Errorf("walking repo: %w", err)
	}

	// Compute index checksum
	indexData, err := json.Marshal(s.index)
	if err != nil {
		return fmt.Errorf("marshaling index: %w", err)
	}

	hash := sha256.Sum256(indexData)
	s.index.IndexSHA256 = hex.EncodeToString(hash[:])

	// Write index to disk
	if err := os.WriteFile(s.indexPath, indexData, 0644); err != nil {
		return fmt.Errorf("writing index: %w", err)
	}

	log.Printf("Index built: %d packages", len(s.index.Packages))
	return nil
}

// ============================================================
// HTTP Handlers
// ============================================================

func (s *Server) handleIndex(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	w.Header().Set("X-OPK-Index-SHA256", s.index.IndexSHA256)

	json.NewEncoder(w).Encode(s.index)
}

func (s *Server) handlePackage(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	firstChar := vars["first"]
	filename := vars["filename"]

	pkgPath := filepath.Join(s.repoDir, "pool", firstChar, filename)

	if _, err := os.Stat(pkgPath); os.IsNotExist(err) {
		http.NotFound(w, r)
		return
	}

	w.Header().Set("Content-Type", "application/octet-stream")
	w.Header().Set("Content-Disposition", fmt.Sprintf("attachment; filename=%s", filename))
	http.ServeFile(w, r, pkgPath)
}

func (s *Server) handleSource(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	firstChar := vars["first"]
	filename := vars["filename"]

	srcPath := filepath.Join(s.repoDir, "source", firstChar, filename)

	if _, err := os.Stat(srcPath); os.IsNotExist(err) {
		http.NotFound(w, r)
		return
	}

	w.Header().Set("Content-Type", "application/gzip")
	http.ServeFile(w, r, srcPath)
}

func (s *Server) handleStats(w http.ResponseWriter, r *http.Request) {
	var totalSize uint64
	for _, pkg := range s.packages {
		totalSize += pkg.PackageSize
	}

	stats := Stats{
		TotalPackages:    len(s.packages),
		TotalArchives:    len(s.packages),
		TotalSizeBytes:   totalSize,
		LastUpdated:      s.index.GeneratedAt,
		RepositoryOrigin: "openos",
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(stats)
}

func (s *Server) handleHealth(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	fmt.Fprintf(w, `{"status":"ok","packages":%d}`, len(s.packages))
}

// ============================================================
// Helpers
// ============================================================

func computeSHA256(path string) (string, uint64, error) {
	f, err := os.Open(path)
	if err != nil {
		return "", 0, err
	}
	defer f.Close()

	h := sha256.New()
	size, err := io.Copy(h, f)
	if err != nil {
		return "", 0, err
	}

	return hex.EncodeToString(h.Sum(nil)), uint64(size), nil
}

func formatBytes(bytes uint64) string {
	const unit = 1024
	if bytes < unit {
		return fmt.Sprintf("%d B", bytes)
	}
	div, exp := uint64(unit), 0
	for n := bytes / unit; n >= unit; n /= unit {
		div *= unit
		exp++
	}
	return fmt.Sprintf("%.1f %cB", float64(bytes)/float64(div), "KMGTPE"[exp])
}

// ============================================================
// Main
// ============================================================

func main() {
	repoDir := flag.String("repo", "/var/opk/repo", "Repository directory")
	addr := flag.String("addr", ":8080", "Listen address")
	flag.Parse()

	// Ensure repo directory structure
	for _, dir := range []string{"pool", "source", "api"} {
		os.MkdirAll(filepath.Join(*repoDir, dir), 0755)
	}

	server := NewServer(*repoDir)
	if err := server.BuildIndex(); err != nil {
		log.Printf("Warning: Could not build index: %v", err)
		log.Println("Server will start but may have an empty index")
	}

	r := mux.NewRouter()

	// API routes
	r.HandleFunc("/Packages.json", server.handleIndex).Methods("GET")
	r.HandleFunc("/pool/{first}/{filename}", server.handlePackage).Methods("GET")
	r.HandleFunc("/source/{first}/{filename}", server.handleSource).Methods("GET")
	r.HandleFunc("/stats", server.handleStats).Methods("GET")
	r.HandleFunc("/health", server.handleHealth).Methods("GET")

	// CORS middleware for web-based clients
	c := cors.New(cors.Options{
		AllowedOrigins:   []string{"*"},
		AllowedMethods:   []string{"GET", "POST", "OPTIONS"},
		AllowedHeaders:   []string{"*"},
		AllowCredentials: true,
	})

	handler := c.Handler(r)

	log.Printf("OPK Repository Server starting on %s", *addr)
	log.Printf("Serving repository: %s", *repoDir)
	log.Printf("Packages.json:        http://localhost%s/Packages.json", *addr)
	log.Printf("Health check:         http://localhost%s/health", *addr)
	log.Printf("Statistics:           http://localhost%s/stats", *addr)

	if err := http.ListenAndServe(*addr, handler); err != nil {
		log.Fatalf("Server failed: %v", err)
	}
}
