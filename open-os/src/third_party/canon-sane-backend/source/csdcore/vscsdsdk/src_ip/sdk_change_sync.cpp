/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include "ipsdk.h"
#include "sdk_image_util.h"
#include "ceilogwrite.h"

long ceisdk_change_sync(ICeiImage **ppInOut, long a)
{
	WriteLog("ceisdk_change_sync() start");
	ICeiImage *pin = *ppInOut;
	long old_sync = pin->sync();
	long new_sync = pin->sync();
	switch (pin->spp()*pin->bps()) {
		case 1:
		new_sync = (pin->width() + 7 ) / 8;
		new_sync = (new_sync + a - 1) / a * a;
		break;
		case 8:
		new_sync = (pin->width() + a - 1) / a  * a;
		break;
		case 24:
		new_sync = (pin->width() * 3 + a - 1) / a  * a;
		break;
	}
	if (old_sync <= new_sync) return 0;

	char *psrc = pin->img();
	char *pdst = pin->img();
	for (long h=0; h<pin->height(); h++) {
		memcpy(pdst, psrc, new_sync);
		psrc+=old_sync;
		pdst+=new_sync;
	}

	CVSCSDSDKImage *pout = create_vscsdsdk_image();
	if (pout) {
		pout->width(pin->width());
		pout->height(pin->height());
		pout->xdpi(pin->xdpi());
		pout->ydpi(pin->ydpi());
		pout->spp(pin->spp());
		pout->bps(pin->bps());
		pout->sync(new_sync);
		pout->attach(pin, pout->sync()*pout->height());
		*ppInOut = pout;
	}
	WriteLog("ceisdk_change_sync() end");
	return 0;		
}