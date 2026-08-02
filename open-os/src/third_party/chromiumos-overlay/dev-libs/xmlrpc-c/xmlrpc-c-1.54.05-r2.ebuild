# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit flag-o-matic multilib-minimal cros-toolchain-funcs

# Upstream maintains 3 release channels: https://xmlrpc-c.sourceforge.net/release.html
# 1. Only the "Super Stable" series is released as a tarball
# 2. SVN tagging of releases seems spotty: https://svn.code.sf.net/p/xmlrpc-c/code/release_number/
# Because of this, we are following the "Super Stable" release channel

DESCRIPTION="A lightweight RPC library based on XML and HTTP"
HOMEPAGE="https://xmlrpc-c.sourceforge.net/"
SRC_URI="mirror://sourceforge/${PN}/${P}.tgz"

LICENSE="BSD"
SLOT="0/4.54"
KEYWORDS="*"
IUSE="abyss +cgi +curl +cxx +libxml2 threads test"
RESTRICT="!test? ( test )"
REQUIRED_USE="test? ( abyss curl cxx )"

RDEPEND="
	sys-libs/ncurses:0=[${MULTILIB_USEDEP}]
	sys-libs/readline:0=[${MULTILIB_USEDEP}]
	curl? ( net-misc/curl[${MULTILIB_USEDEP}] )
	libxml2? ( dev-libs/libxml2[${MULTILIB_USEDEP}] )
"
DEPEND="${RDEPEND}"
# TODO(b/376513486): Upstream the net-libs/curl dependency.
# Without it, we get bazel build failures because it's an implicit
# dependency.
BDEPEND="
	virtual/pkgconfig
	curl? ( net-misc/curl[${MULTILIB_USEDEP}] )
"

PATCHES=(
	"${FILESDIR}"/${PN}-1.51.06-pkg-config-libxml2.patch
	"${FILESDIR}"/${PN}-1.51.06-pkg-config-openssl.patch
)

pkg_setup() {
	use curl || ewarn "Curl support disabled: No client library will be built"
}

src_prepare() {
	default

	sed -i \
		-e "/CFLAGS_COMMON/s|-g -O3$||" \
		-e "/CXXFLAGS_COMMON/s|-g$||" \
		common.mk || die

	# Out-of-source install phase is broken
	multilib_copy_sources
}

multilib_src_configure() {
	tc-export PKG_CONFIG

	# TODO(b/376513486): Needed to compile with libcxx versions that still contain
	# std::auto_ptr. Remove this when this package is upgraded to 1.58 or later.
	append-cxxflags "-std=c++14"

	ECONF_SOURCE="${S}" \
	econf \
		--disable-libwww-client \
		--disable-wininet-client \
		--without-libwww-ssl \
		$(use_enable abyss abyss-server) \
		$(use_enable cgi cgi-server) \
		$(use_enable curl curl-client) \
		$(use_enable cxx cplusplus) \
		$(use_enable libxml2 libxml2-backend) \
		$(use_enable threads abyss-threads)
}

multilib_src_compile() {
	cros_enable_cxx_exceptions

	default_src_compile
	# Tools building is broken in this release
	#multilib_is_native_abi && use tools && emake -rC "${S}"/tools
}

multilib_src_test() {
	# Needed for tests, bug #836469
	cp "${BUILD_DIR}"/include/xmlrpc-c/config.h "${S}"/include/xmlrpc-c || die
	default_src_test
}
