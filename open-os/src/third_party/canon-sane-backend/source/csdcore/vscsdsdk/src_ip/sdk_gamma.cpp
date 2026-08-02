/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "ipsdk.h"
#include "sdk_image_util.h"
#include "global_apis.h"
#include "ceilogwrite.h"

namespace {
#ifdef __ANDROID__
    void* android_fopen(void *hm, const char* path, const char* mode)
    {
        void* out = NULL;
        typedef void *(*lpfnfopen)(const char* path, const char* mode);
        lpfnfopen lpfn = (lpfnfopen)dlsym(hm, "android_fopen");
        if (lpfn) {
            out = lpfn(path, mode);
        }
        return out;
    }
    void android_fclose(void *hm, void* fp)
    {
        typedef void (*lpfnfclose)(void* fp);
        lpfnfclose lpfn = (lpfnfclose)dlsym(hm, "android_fclose");
        if (lpfn) {
            lpfn(fp);
        }
    }
    bool FileFound(const char *fname)
    {
        bool out = false;
        void* hm = dlopen("libceiandroid_assets.so", RTLD_LAZY);
        if (hm == NULL) {
            return false;
        }
        char *ppath = strrchr((char*)fname, '/');
        if (ppath) ppath++;
        else ppath = (char*)fname;
        void *fp = android_fopen(hm, ppath, "r");
        if (fp) {
            android_fclose(hm, fp);
            out = true;
        }
        dlclose(hm);
        return out;
    }
#else
	bool FileFound(const char *fname)
	{
		bool out = false;
		FILE *fp = fopen(fname, "r");
		if (fp) {
			fclose(fp);
			out = true;
		}
		return out;
	}
#endif
    bool no_ini_file()
    {
        char path[256] = { 0 };
        bool out = !FileFound(ceisdk_get_ini_path(path, sizeof(path)));
        if (out) {
            printf("%s is not found\r\n", path);
        }
        return out;
    }
	#if 0
	void print_gamma(unsigned char * p, unsigned int size)
	{
		for (unsigned int i = 0; i < size; i++) {
			printf("%d,", p[i]);
			if ((i & 7) == 7) {
				printf("\r\n");
			}
		}
	}
	#endif
	long cei_max(long a, long b) { return (((a) > (b)) ? (a) : (b));}
	long cei_min(long a, long b) { return (((a) < (b)) ? (a) : (b)); }
	double get_Z(long low/*-128*/, long mid/*0*/, long high/*128*/, long brightness)
	{
		double increment = 0;
		double base = 0;
		double addition = 0;
		if (brightness < 128) {
			increment = (double)(mid - low) / (double)(128 - 1);
			base = low;
			addition = brightness - 1;
		}
		else if (brightness == 128) {
			increment = 0;
			base = mid;
			addition = 0;
		}
		else {
			increment = (double)(high - mid) / (double)(255 - 128);
			base = mid;
			addition = brightness - 128;
		}
		return increment * addition + base;
	}
	long ceisdk_get_gamma_table_OldType(const char* section, unsigned char* gamma, long gamma_size, long brightness, long contrast)
	{
		char keyname[8] = { 0 };

		double K = ceisdk_get_private_profile_double(section, ("K"), 437.0);
		long low = ceisdk_get_private_profile_int(section, ("Z_low"), -128);
		long mid = ceisdk_get_private_profile_int(section, ("Z_mid"), 0);
		long high = ceisdk_get_private_profile_int(section, ("Z_high"), 128);
		double Z = get_Z(low, mid, high, brightness);
		double k2 = ceisdk_get_private_profile_double(section, ("k2"), 2.2);
		sprintf(keyname, ("k1_%ld"), contrast);
		double k1 = ceisdk_get_private_profile_double(section, keyname, 1.0);
		sprintf(keyname, ("D_%ld"), contrast);
		double D = ceisdk_get_private_profile_double(section, keyname, 1.0);
		sprintf(keyname, ("X_%ld"), contrast);

		enum {
			GAMMA_SIZE = 256
		};

		unsigned char* gmm = gamma;

		//�w��
		for (int x = 0; x < GAMMA_SIZE; x++) {

			double y = k1 * K * pow((double)x / 255.0, 1.0 / k2) + D + Z + 0.5;
			gmm[x] = (int)cei_min(255, cei_max(0, (long)y));
		}

		return 0;
	}
	long ceisdk_get_gamma_table_DRChipType(const char* section, unsigned char* gamma, long gamma_size, long brightness, long contrast)
	{
		char keyname[8] = { 0 };

		double K = ceisdk_get_private_profile_double(section, ("K"), 437.0);
		long low = ceisdk_get_private_profile_int(section, ("Z_low"), -128);
		long mid = ceisdk_get_private_profile_int(section, ("Z_mid"), 0);
		long high = ceisdk_get_private_profile_int(section, ("Z_high"), 128);
		double Z = get_Z(low, mid, high, brightness);
		double k2 = ceisdk_get_private_profile_double(section, ("k2"), 2.2);
		sprintf(keyname, ("k1_%ld"), contrast);
		double k1 = ceisdk_get_private_profile_double(section, keyname, 1.0);
		sprintf(keyname, ("D_%ld"), contrast);
		double D = ceisdk_get_private_profile_double(section, keyname, 1.0);
		sprintf(keyname, ("X_%ld"), contrast);
		double X = ceisdk_get_private_profile_double(section, keyname, 1.0);
		sprintf(keyname, ("Y_%ld"), contrast);
		double Y = ceisdk_get_private_profile_double(section, keyname, 1.0);
		sprintf(keyname, ("a_%ld"), contrast);
		double a = ceisdk_get_private_profile_double(section, keyname, 1.0);
		double b = Y - X * a;

		enum {
			GAMMA_SIZE = 256
		};

		unsigned char* gmm = gamma;
		unsigned char gmm1[GAMMA_SIZE] = { 0 };

		for (int x = 0; x < GAMMA_SIZE; x++) {
			double y = a * (x - Z) + b + 0.5;
			gmm1[x] = (int)cei_min(255, cei_max(0, (long)y));
		}

		for (int x = 0; x < GAMMA_SIZE; x++) {

			double y = 0.0;
			if (((double)x - Z) > 0.0) {
				y = k1 * K * pow(((double)x - Z) / 255.0, 1.0 / k2) + D + 0.5;
			}
			gmm[x] = (int)cei_min(255, cei_max(0, (long)y));
		}

		for (int x = 0; x < GAMMA_SIZE; x++) {
			gmm[x] = (int)cei_max(gmm[x], gmm1[x]);
		}

		return 0;
	}

	long ceisdk_get_gamma_table_(const char *section, unsigned char *gamma, long gamma_size, long brightness, long contrast)
	{
		if (no_ini_file()) {
			WriteLog("ini file is not found\r\n");
			return -1;
		}

		if (brightness <= 0) brightness = 128;
		brightness = cei_min(255, cei_max(0, brightness));

		if (contrast <= 0) { contrast = 4; }
		if (contrast > 7) { contrast = contrast * 7 / 255 + 1; }
		contrast = cei_min(7, cei_max(1, contrast));

		int gmm_type = ceisdk_get_private_profile_int(section, ("gmm_type"), 0);
		if (gmm_type == 0) {
			return ceisdk_get_gamma_table_DRChipType(section, gamma, gamma_size, brightness, contrast);
		}
		else if (gmm_type == 1) {
			return ceisdk_get_gamma_table_OldType(section, gamma, gamma_size, brightness, contrast);
		}

		return -1;


	}
	long ceisdk_get_table_picture_core(unsigned char* gmm, int gammaLen, long nBr, long nCr)
	{
		if (no_ini_file()) {
			WriteLog("ini file is not found\r\n");
			return -1;
		}
		const char * section = "MSS_Picture";
		if (nCr <= 0) { nCr = 4; }
		if (nCr > 7) { nCr = nCr * 7 / 255 + 1; }
		nCr = cei_min(7, cei_max(1, nCr));

		int x_shift = 3; 
		int x_saturation = 240; 
		int x_conversion = 180; 
		char keyname[8] = { 0 };
		sprintf(keyname, ("gain_%ld"), nCr);
		double gain = ceisdk_get_private_profile_double(section, keyname, 0);
		//const double gain_table[] = { -1, -0.375, -0.25, -0.125, 0, 0.25, 0.5, 0.75 }; 

		double y_on_min = pow((double)x_shift / 255, 1 / 2.2);
		double y_on_max = pow((double)(x_saturation + x_shift) / 255, 1 / 2.2);
		double y_on_conversion = pow((double)(x_conversion + x_shift) / 255, 1 / 2.2);

		double K = 255;
		double z = ((double)nBr - 128) * 128 / 127 + 0;
		double k = 1.0 / (y_on_max - y_on_min) + gain;
		double d = -1.0 * K * y_on_min / (y_on_max - y_on_min) - K * y_on_conversion * gain;

		for (int x = 0; x < gammaLen; x++) {
			double y = K * k * pow((double)(x + x_shift) / 255, 1 / 2.2) + d + z + 0.5;
			gmm[x] = (unsigned char)(y > 255 ? 255 : (y < 0 ? 0 : y));
		}
		return 0;
	}
	long ceisdk_get_table_mix_common_core(unsigned char* gmm, long gammaLen, long nBr, long nCr)
	{
		if (nCr <= 0) { nCr = 4; }
		if (nCr > 7) { nCr = nCr * 7 / 255 + 1; }
		nCr = cei_min(7, cei_max(1, nCr));

		int x_shift = 3;
		int x_saturation = 240;
		int x_conversion = 180; 
		const double gain_table[] = { -1, -0.375, -0.25, -0.125, 0, 0.25, 0.5, 0.75 }; 
		double gain = gain_table[nCr];

		double y_on_min = pow((double)x_shift / 255, 1 / 2.2);
		double y_on_max = pow((double)(x_saturation + x_shift) / 255, 1 / 2.2);
		double y_on_conversion = pow((double)(x_conversion + x_shift) / 255, 1 / 2.2);

		double K = 255;
		double z = ((double)nBr - 128) * 128 / 127 + 0;
		double k = 1.0 / (y_on_max - y_on_min) + gain;
		double d = -1.0 * K * y_on_min / (y_on_max - y_on_min) - K * y_on_conversion * gain;

		for (int x = 0; x < gammaLen; x++) {
			double y = K * k * pow((double)(x + x_shift) / 255, 1 / 2.2) + d + z + 0.5;
			gmm[x] = (unsigned char)(y > 255 ? 255 : (y < 0 ? 0 : y));
		}
		return 0;
	}
	long ceisdk_get_table_picture_common_core(unsigned char* gmm, long gammaLen, long nBr, long nCr)
	{
		if (nCr <= 0) { nCr = 4; }
		if (nCr > 7) { nCr = nCr * 7 / 255 + 1; }
		nCr = cei_min(7, cei_max(1, nCr));

		const double k1table[] = {0, 0.62, 0.74, 0.86,0.98,1.20,1.42,1.63};
		const double k1 = k1table[nCr];
		const double K = 300;
		const int Dtable[] = {0, 65, 33, 1, -31, -69, -146, -203};
		const int D = Dtable[nCr];
		const double Z = ((double)nBr - 128) * 128 / 127 + 0;

		double atable[] = { 0,1.98,2.15,2.31,2.50,2.31,2.29,2.40};
		double a = atable[nCr];
		double b = k1 * K * pow((48.0 / 255.0), (1 / 2.5)) + D + Z;

		for (int x = 0; x < gammaLen; x++) {
			double y = 0;
			if (x <= 48) {
				y = b - a * (48 - x);
			}
			else {
				y = k1 * K * pow((x / 255.0), (1 / 2.5)) + D + Z;
			}
			gmm[x] = (unsigned char)(y > 255 ? 255 : (y < 0 ? 0 : y));
		}
		//print_gamma(gmm, gammaLen);
		return 0;
	}

	bool convert_inv_gamma(unsigned char* gmm, long gamma_size)
	{
		if (!gmm) return false;
		if (gamma_size != 256) return false;
		unsigned char gmm_src[256] = { 0 };
		memcpy(gmm_src, gmm, 256 * sizeof(unsigned char));

		long t = 0;
		for (long s = 0; s < gamma_size; s++) {
			long end = (gamma_size - 1) < gmm_src[s] ? (gamma_size - 1) : gmm_src[s];
			for (long n = t; n <= end; n++) {
				gmm[n] = (unsigned char)(s > 255 ? 255 : (s < 0 ? 0 : s));
			}
			t = end + 1;
		}
		for (long m = t; m < gamma_size; m++) {
			gmm[m] = 255;
		}
		return true;
	}

	bool ceisdk_get_table_shading(unsigned char* gmm, long gamma_size, unsigned int target)
	{
		if (!gmm) return false;
		if (gamma_size != 256) return false;
		for (unsigned int i = 0; i < 256; i++) {
			unsigned int s = i * target / 256;
			gmm[i] = (unsigned char)(s > 255 ? 255 : s);
		}
		return true;
	}
}
long ceisdk_get_gamma_table_color(unsigned char *gamma, long gamma_size, long brightness, long contrast)
{
	WriteLog("ceisdk_get_gamma_table_color(gamma, %d, %d, %d) start", gamma_size, brightness, contrast);
	long out =  ceisdk_get_gamma_table_("MSS_Color", gamma, gamma_size, brightness, contrast);
	WriteLog("ceisdk_get_gamma_table_color() end");
	return out;
}
long ceisdk_get_gamma_table_gray(unsigned char *gamma, long gamma_size, long brightness, long contrast)
{
	WriteLog("ceisdk_get_gamma_table_gray(gamma, %d, %d, %d) start", gamma_size, brightness, contrast);
	long out = ceisdk_get_gamma_table_("MSS_Gray", gamma, gamma_size, brightness, contrast);
	WriteLog("ceisdk_get_gamma_table_gray() end");
	return out;
}
long ceisdk_get_gamma_table_bw(unsigned char *gamma, long gamma_size, long brightness, long contrast)
{
	WriteLog("ceisdk_get_gamma_table_bw(gamma, %d, %d, %d) start", gamma_size, brightness, contrast);
	if (no_ini_file()) {
		WriteLog("ini file is not found\r\n");
		return -1;
	}
	if (brightness <= 0) brightness = 128;
	brightness = cei_min(255, cei_max(0, brightness));
	
	if (contrast <= 0) { contrast = 4; }
	if (contrast > 7) { contrast = contrast * 7 / 255 + 1; }
	contrast = cei_min(7, cei_max(1, contrast));

	char keyname[8] = { 0 };

	double S = ceisdk_get_private_profile_double(("MSS_Bin"), ("S"), 282.0);
	sprintf(keyname, ("k_%ld"), contrast);
	double k = ceisdk_get_private_profile_double(("MSS_Bin"), keyname, 1.0);
	sprintf(keyname, ("Z_%ld"), contrast);
	double Z = ceisdk_get_private_profile_double(("MSS_Bin"), keyname, 0.0);
	unsigned char *gmm = gamma;
	long max = gamma_size;
	for (long x = 0; x < max; x++) {
		double y = S * pow(((double)x) / 255.0, 1.0 / k) + Z + 0.5;
		gmm[x] = (unsigned char)cei_min(255, cei_max(0, (long)y));
	}
	WriteLog("ceisdk_get_gamma_table_bw() end");
	return 0;
}
long ceisdk_get_gamma_table_dither(unsigned char *gamma, long gamma_size, long brightness, long contrast)
{
	WriteLog("ceisdk_get_gamma_table_dither(gamma, %d, %d, %d) start", gamma_size, brightness, contrast);
	if (no_ini_file()) {
		WriteLog("ini file is not found\r\n");
		return -1;
	}
	if (brightness <= 0) brightness = 128;
	brightness = cei_min(255, cei_max(0, brightness));

	if (contrast <= 0) { contrast = 4; }
	if (contrast > 7) { contrast = contrast * 7 / 255 + 1; }
	contrast = cei_min(7, cei_max(1, contrast));

	char keyname[8] = { 0 };

	sprintf(keyname, ("k1_%ld"), contrast);
	double k1 = ceisdk_get_private_profile_double(("MSS_Dither"), keyname, 1.0);
	double k = (double)ceisdk_get_private_profile_double(("MSS_Dither"), ("k"), 388.0);
	double k2 = ceisdk_get_private_profile_double(("MSS_Dither"), ("k2"), 2.2);
	sprintf(keyname, ("D_%ld"), contrast);
	double D = ceisdk_get_private_profile_double(("MSS_Dither"), keyname, 82.0);

	long low = ceisdk_get_private_profile_int(("MSS_Dither"), ("Z_low"), -128);
	long mid = ceisdk_get_private_profile_int(("MSS_Dither"), ("Z_mid"), 0);
	long high = ceisdk_get_private_profile_int(("MSS_Dither"), ("Z_high"), 128);
	
	double Z = get_Z(low, mid, high, 256 - brightness);
	double z1 = ceisdk_get_private_profile_double(("MSS_Dither"), ("z1"), -70);
	
	unsigned char * gmm = gamma;
	enum {
		GAMMA_SIZE = 256
	};
	for (int x = 0; x < GAMMA_SIZE; x++) {
		double y = k1 * k * pow(((double)x) / 255.0, 1.0 / k2) + D + Z + z1 + 0.5;
		gmm[x] = (unsigned char)cei_min(255, cei_max(0, (long)y));
	}
	WriteLog("ceisdk_get_gamma_table_dither() end");
	return 0;
}
long ceisdk_gamma(ICeiImage *pin, unsigned char *gamma, long gamma_size)
{
	WriteLog("ceisdk_gamma() start");
	write_debug_bmp_file("gmm_in", pin);
	if (pin->comptype()) {
		WriteLog("invalid comp type\r\n");
		return -1;
	}
	if (pin->bps() == 1) {
		WriteLog("invalid bps\r\n");
		return -1;
	}
	unsigned char *ptr = (unsigned char*)pin->img();
	long max = pin->size();
	for (long i = 0; i < max; i++) {
		ptr[i] = gamma[ptr[i]];
	}
	write_debug_bmp_file("gmm_out", pin);
	WriteLog("ceisdk_gamma() end");
	return 0;
}
long ceisdk_gamma(ICeiImage *pin, LPFN_CEISDK_GAMMA_TABLE lpfn, long brightness, long contrast)
{
	WriteLog("ceisdk_gamma(pin, lpfn, %d, %d) start", brightness, contrast);
	if (pin->comptype()) return -1;
	if (pin->bps() == 1) return -1;
	unsigned char gamma[256];
	for (long i = 0; i < 256; i++) { gamma[i] = (unsigned char)i; }
	lpfn(gamma, 256, brightness, contrast);
	long out = ceisdk_gamma(pin, gamma, 256);
	WriteLog("ceisdk_gamma() end");
	return out;
}

long ceisdk_gamma(ICeiImage *pin, unsigned char *gamma_r, unsigned char *gamma_g, unsigned char *gamma_b, long gamma_size)
{
	if (pin->comptype()) return -1;
	if (pin->bps() == 1) return -1;
	if (pin->spp() != 3) return -1;
	if (!gamma_r || !gamma_g || !gamma_b || gamma_size != 256) return -1;

	unsigned char *ptr = (unsigned char*)pin->img();
	unsigned char *ptrend = ptr + pin->size();
	unsigned int width = pin->width();
	while(ptr < ptrend) {
		for (unsigned int x = 0; x < width; x++) {
			ptr[x * 3 + 0] = gamma_r[ptr[x * 3 + 0]];
			ptr[x * 3 + 1] = gamma_g[ptr[x * 3 + 1]];
			ptr[x * 3 + 2] = gamma_b[ptr[x * 3 + 2]];
		}
		ptr += pin->sync();
	}
	return 0;
}
