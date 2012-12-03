#include "OSAL_api.h"
#include "com_type.h"
#include "pe_api.h"
#include "shm_api.h"
#include "gfx_common.h"
#include "com_type.h"
#include "ErrorCode.h"
#include "setplane.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>		//to use atoi

#define MAX_CMD_LEN       80
#define MAX_ARG_CNT       10
#define SetPlane_PROMPT            "ITOOLS:$ =>SET_PLANE:$ "
#define SetPlane_REV         "1.0"
#define INVALID_ARGUMENT  printf("Invalid argument!\n")

void *hPE = NULL;
int channel_id;
void * hGFX=NULL;
MV_PE_GFX_HCEL hCel=NULL;
char plane_names[7][32] = { 
{"MV_PE_CHANNEL_PRIMARY_VIDEO"},
{"MV_PE_CHANNEL_SECONDARY_VIDEO"},
{"MV_PE_CHANNEL_GRAPHICS(AWT)"},
{"MV_PE_CHANNEL_CURSOR"},
{"MV_PE_CHANNEL_MOSD"},
{"MV_PE_CHANNEL_SUBTITLE"},
{"MV_PE_CHANNEL_BACKGROUND"}
};

static int get_plane_information()
{
	int ret=0;
	unsigned int zOrder;
	unsigned char Visible;

	int plane_id;
        for(plane_id=0; plane_id <7; plane_id++){
        	printf("plane id=%d, %s\n", plane_id+1, plane_names[plane_id]);
		ret = MV_PE_VideoGetZOrder(hPE, plane_id+1, &zOrder);
	        if (ret){
        	        printf("MV_PE_VideoGetZOrder faild, ret=0x%x\n", ret);
                	return ret;
	        }
        	printf("    zorder =%u\n", zOrder);

	        ret = MV_PE_VideoGetVisible(hPE, plane_id+1,&Visible);
        	if (ret){
	                printf("MV_PE_VideoGetVisible faild, ret=0x%x\n", ret);
	                return ret;
	        }
		printf("    visible=%u\n", Visible);
	}
	
	return 0;
}
static int pe_suspend()
{
	int ret = 0;

	ret = MV_PE_PowerSuspend(hPE);
	if (ret){
		printf("MV_PE_PowerSuspend faild, ret=0x%x\n", ret);
		return ret;
	}
	printf("MV_PE_PowerSuspendu success");

	return 0;
}

static int pe_resume()
{
	int ret = 0;

	ret = MV_PE_PowerResume(hPE);
	if (ret){
		printf("MV_PE_PowerResume faild, ret=0x%x\n", ret);
		return ret;
	}
	printf("MV_PE_PowerResume success\n");

	return 0;
}

static int get_scaling()
{
	int ret = 0;
	int plane_id=1;
        int src, dst;
        MV_PE_GFX_RECT srcW={0,0,0,0}, dstW={0,0,0,0};

	printf("Please input plane id:\n");
        scanf("%d", &plane_id);

	ret = MV_PE_VideoGetScaling(hPE, plane_id, &srcW, &dstW);
	if (ret){
		printf("MV_PE_VideoGetScaling faild, ret=0x%x\n", ret);
		return ret;
	}
	printf("plane_id=%d, src_window={%d, %d, %d*%d}, dst_window={%d, %d, %d,%d}\n", 
        plane_id, srcW.x, srcW.y, srcW.w, srcW.h, dstW.x, dstW.y, dstW.w, dstW.h);
        return 0;
}

static int set_scaling()
{
	int ret = 0;
	int plane_id;
        int src, dst;
        MV_PE_GFX_RECT srcW={0,0,0,0}, dstW={0,0,0,0};

	printf("Please input plane id:\n");
	scanf("%d", &plane_id);
	printf("Please input src window:\n1 for 480\n2 for 720\n 3 for 128*128\n 4 for 8*8\n others for 1080\n");
	scanf("%d", &src);
  if (src == 1){
		srcW.w = 720;
		srcW.h = 480;
	}
	else if (src == 2){
		srcW.w = 1280;
		srcW.h = 720;
	}
	else if (src == 3){
		srcW.w = 128;
		srcW.h = 128;
	}
	else if (src == 4){
		srcW.w = 8;
		srcW.h = 8;
	}
	else{
		srcW.w = 1920;
		srcW.h = 1080;
	}
	printf("Please input dst window:\n1 for 480\n2 for 720\n 3 for 128*128\n 4 for 8*8\n others for 1080\n");
	scanf("%d", &dst);
  if (dst == 1){
		dstW.w = 720;
		dstW.h = 480;
	}
	else if (dst == 2){
		dstW.w = 1280;
		dstW.h = 720;
	}
	else if (dst == 3){
		dstW.w = 128;
		dstW.h = 128;
	}
	else if (dst == 4){
		dstW.w = 8;
		dstW.h = 8;
	}
	else{
		dstW.w = 1920;
		dstW.h = 1080;
	}
	printf("plane_id=%d, src_window={%d*%d}, dst_window={%d,%d}\n", plane_id, srcW.w, srcW.h, dstW.w, dstW.h);

	ret = MV_PE_VideoSetScaling(hPE, plane_id, &srcW, &dstW);
	if (ret){
		printf("MV_PE_VideoSetScaling faild, ret=0x%x\n", ret);
		return ret;
	}
	return ret;
}

static int set_plane_visible()
{
	int ret = 0;
	int plane_id;
       unsigned char visible;

	printf("Please input plane id:\n");
	scanf("%d", &plane_id);
	printf("Please input visible:\n");
	scanf("%u", &visible);
	printf("planed id=%d, visible=%u\n", plane_id, visible);

	ret = MV_PE_VideoSetVisible(hPE, plane_id, visible);
	if (ret){
		printf("MV_PE_VideoSetVisible faild, ret=0x%x\n", ret);
		return ret;
	}
	ret = MV_PE_VideoGetVisible(hPE, plane_id, &visible);
	if (ret){
		printf("MV_PE_VideoGetVisible faild, ret=0x%x\n", ret);
		return ret;
	}
	printf("after set, visible=%u\n", visible);
	return ret;
}

static int swap_zorder()
{
	int ret=0;
	int bottom_id, top_id;
	int bottom_zorder, top_zorder;

	printf("Please input plane-1 id:\n");
	scanf("%d", &bottom_id);
        printf("Please input plane-2 id:\n");
        scanf("%u", &top_id);
        ret = MV_PE_VideoGetZOrder(hPE, bottom_id, &bottom_zorder);
        if (ret){
                printf("MV_PE_VideoGetZOrder faild, ret=0x%x\n", ret);
                return ret;
        }

	ret = MV_PE_VideoGetZOrder(hPE, top_id, &top_zorder);
        if (ret){
                printf("MV_PE_VideoGetZOrder faild, ret=0x%x\n", ret);
                return ret;
        }
        printf("planed-1 id=%d, zorder=%d\n", bottom_id, bottom_zorder);
        printf("planed-2 id=%d, zorder=%d\n", top_id, top_zorder);
	MV_PE_VideoSetZOrder(hPE, top_id, bottom_zorder);
	MV_PE_VideoSetZOrder(hPE, bottom_id, top_zorder);
        
	ret = MV_PE_VideoGetZOrder(hPE, bottom_id, &bottom_zorder);
        if (ret){
                printf("MV_PE_VideoGetZOrder faild, ret=0x%x\n", ret);
                return ret;
        }

	ret = MV_PE_VideoGetZOrder(hPE, top_id, &top_zorder);
        if (ret){
                printf("MV_PE_VideoGetZOrder faild, ret=0x%x\n", ret);
                return ret;
        }
        printf("after swap\nplaned-1 id=%d, zorder=%d\n", bottom_id, bottom_zorder);
        printf("planed-2 id=%d, zorder=%d\n", top_id, top_zorder);

	return ret;
}

static int draw_2d_graphics()
{
	unsigned int plane_id;
	int ret=0;
	GFX_RECT	srcRect={0,0,0,0};
	GFX_RECT	dstRect={0,0,0,0};
	MV_PE_GFX_SIZE	size;
	MV_PE_GFX_CelInfo celInfo;

	printf("Please input plane id:\n");
	scanf("%u", &plane_id);
	ret = MV_PE_GFX_Open(hPE, plane_id, "LAYER2", &hGFX);
	if (ret ||(!hGFX)){
	    printf("open gfx error, ret=0x%x\n",ret);
            return ret;
	}

	size = MAKE_SIZE(1920,1080);
        ret = MV_PE_GFX_CreateCel(hGFX, NULL, MV_PE_GFX_PIX_A8R8G8B8, size, &hCel);
	if (ret ||(!hCel)){
	    printf("create cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	ret=MV_PE_GFX_FillCel(hGFX, hCel, 0xFFFFFFFF, NULL, NULL, NULL);
	if (ret ||(!hCel)){
	    printf("fill cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	ret = MV_PE_GFX_GetCelInfo(hGFX, hCel, &celInfo);
	if (ret){
	    printf("get celinfo error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	srcRect.w = dstRect.w = 1920;
	srcRect.h = dstRect.h = 1080;
	ret = MV_PE_GFX_DispImg(hGFX, celInfo.hImage, &srcRect, &dstRect, plane_id, NULL, NULL);
	if (ret){
	    printf("display image error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	usleep(1000000);
	ret=MV_PE_GFX_FillCel(hGFX, hCel, 0xFF00FF00, NULL, NULL, NULL);
	if (ret ||(!hCel)){
	    printf("fill cel error2, ret=0x%x\n",ret);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	ret = MV_PE_GFX_DispImg(hGFX, celInfo.hImage, &srcRect, &dstRect, plane_id, NULL, NULL);
	if (ret){
	    printf("display image error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	MV_PE_GFX_DestroyObject(hGFX,hCel);
	MV_PE_GFX_Close(hGFX);

        return ret;
}

static int draw_null_image()
{
	unsigned int plane_id;
	int ret=0;
	GFX_RECT	srcRect={0,0,0,0};
	GFX_RECT	dstRect={0,0,0,0};
	MV_PE_GFX_SIZE	size;
	MV_PE_GFX_CelInfo celInfo;

	printf("Please input plane id:\n");
	scanf("%u", &plane_id);
	ret = MV_PE_GFX_Open(hPE, plane_id, "LAYER2", &hGFX);
	if (ret ||(!hGFX)){
	    printf("open gfx error, ret=0x%x\n",ret);
      return ret;
	}
	ret = MV_PE_GFX_DispImg(hGFX, NULL, NULL, NULL, plane_id, NULL, NULL);
	if (ret){
	    printf("display image error, ret=0x%x\n",ret);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
      return ret;
	}

	MV_PE_GFX_Close(hGFX);

  return ret;
}

static int draw_w_h_by_gpu2d()
{
	unsigned int plane_id;
	int ret=0, w, h;
	GFX_RECT	srcRect={0,0,0,0};
	GFX_RECT	dstRect={0,0,0,0};
	MV_PE_GFX_SIZE	size;
	MV_PE_GFX_CelInfo celInfo;
	MV_PE_GFX_RECT rect;

	printf("Please input plane id:\n");
	scanf("%u", &plane_id);
	ret = MV_PE_GFX_Open(hPE, plane_id, "LAYER2", &hGFX);
	if (ret ||(!hGFX)){
	    printf("open gfx error, ret=0x%x\n",ret);
            return ret;
	}
	
	printf("Please input w:\n");
	scanf("%d", &w);
	printf("Please input h:\n");
	scanf("%d", &h);
	if ((w <= 0)||(h <=0)||(w >1920)||(h >1080)){
		printf("invalide w=%d, h=%d\n",w, h);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	size = MAKE_SIZE(w,h);
  ret = MV_PE_GFX_CreateCel(hGFX, NULL, MV_PE_GFX_PIX_A8R8G8B8, size, &hCel);
	if (ret ||(!hCel)){
	    printf("create cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

  rect.x = w/4;
  rect.y = h/4;
  rect.w = w/2;
  rect.h = h/2;
	ret=MV_PE_GFX_FillCel(hGFX, hCel, 0x77FFFFFF, &rect, NULL, NULL);
	if (ret ||(!hCel)){
	    printf("fill cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	
	rect.x = 0;
  rect.y = 0;
  rect.w = w/2;
  rect.h = h/2;
	ret=MV_PE_GFX_FillCel(hGFX, hCel, 0x7700FF00, &rect, NULL, NULL);
	if (ret ||(!hCel)){
	    printf("fill cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	
	rect.x = 0;
  rect.y = h/2;
  rect.w = w/2;
  rect.h = h/2;
	ret=MV_PE_GFX_FillCel(hGFX, hCel, 0x770000FF, &rect, NULL, NULL);
	if (ret ||(!hCel)){
	    printf("fill cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	
	rect.x = w/2;
  rect.y = 0;
  rect.w = w/2;
  rect.h = h/2;
	ret=MV_PE_GFX_FillCel(hGFX, hCel, 0x77FF0000, &rect, NULL, NULL);
	if (ret ||(!hCel)){
	    printf("fill cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	
	rect.x = w/2;
  rect.y = h/2;
  rect.w = w/2;
  rect.h = h/2;
	ret=MV_PE_GFX_FillCel(hGFX, hCel, 0x77FFFF00, &rect, NULL, NULL);
	if (ret ||(!hCel)){
	    printf("fill cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	ret = MV_PE_GFX_GetCelInfo(hGFX, hCel, &celInfo);
	if (ret){
	    printf("get celinfo error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	srcRect.w = dstRect.w = w;
	srcRect.h = dstRect.h = h;
	ret = MV_PE_GFX_DispImg_Resize(hGFX, celInfo.hImage, &srcRect, &dstRect,  &dstRect, plane_id, NULL, NULL);
	if (ret){
	    printf("display image error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
      return ret;
	}

  usleep(10000000);
	MV_PE_GFX_DestroyObject(hGFX,hCel);
	MV_PE_GFX_Close(hGFX);

        return ret;
}

static int draw_w_h_no_gpu()
{
	unsigned int plane_id;
	int ret=0, w, h, i,j;
	int *pData, *pRowData;
	GFX_RECT	srcRect={0,0,0,0};
	GFX_RECT	dstRect={0,0,0,0};
	MV_PE_GFX_SIZE	size;
	MV_PE_GFX_CelInfo celInfo;
	MV_PE_GFX_RECT rect;

	printf("Please input plane id:\n");
	scanf("%u", &plane_id);
	ret = MV_PE_GFX_Open(hPE, plane_id, "LAYER2", &hGFX);
	if (ret ||(!hGFX)){
	    printf("open gfx error, ret=0x%x\n",ret);
            return ret;
	}
	
	printf("Please input w:\n");
	scanf("%d", &w);
	printf("Please input h:\n");
	scanf("%d", &h);
	if ((w <= 0)||(h <=0)||(w >1920)||(h >1080)){
		printf("invalide w=%d, h=%d\n",w, h);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	size = MAKE_SIZE(w,h);
  ret = MV_PE_GFX_CreateCel(hGFX, NULL, MV_PE_GFX_PIX_A8R8G8B8, size, &hCel);
	if (ret ||(!hCel)){
	    printf("create cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	ret = MV_PE_GFX_GetCelInfo(hGFX, hCel, &celInfo);
	if (ret){
	    printf("get celinfo error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	
	rect.x = w/4;
  rect.y = h/4;
  rect.w = w/2;
  rect.h = h/2;
  pRowData = (int *)celInfo.base + rect.y*w + rect.x ;
	for (i= rect.x; i < rect.x+rect.w; i++){
		pData = pRowData;
		for (j=rect.y; j<rect.y+rect.h; j++){
			*pData++ = 0x77FFFFFF;
		}
		pRowData += w;
	}
	
	rect.x = 0;
  rect.y = 0;
  rect.w = w/2;
  rect.h = h/2;
	pRowData = (int *)celInfo.base + rect.y*w + rect.x ;
	for (i= rect.x; i < rect.x+rect.w; i++){
		pData = pRowData;
		for (j=rect.y; j<rect.y+rect.h; j++){
			*pData++ = 0x7700FF00;
		}
		pRowData += w;
	}
	
	rect.x = 0;
  rect.y = h/2;
  rect.w = w/2;
  rect.h = h/2;
	pRowData = (int *)celInfo.base + rect.y*w + rect.x ;
	for (i= rect.x; i < rect.x+rect.w; i++){
		pData = pRowData;
		for (j=rect.y; j<rect.y+rect.h; j++){
			*pData++ = 0x770000FF;
		}
		pRowData += w;
	}
	
	rect.x = w/2;
  rect.y = 0;
  rect.w = w/2;
  rect.h = h/2;
	pRowData = (int *)celInfo.base + rect.y*w + rect.x ;
	for (i= rect.x; i < rect.x+rect.w; i++){
		pData = pRowData;
		for (j=rect.y; j<rect.y+rect.h; j++){
			*pData++ = 0x77FF0000;
		}
		pRowData += w;
	}
	
	rect.x = w/2;
  rect.y = h/2;
  rect.w = w/2;
  rect.h = h/2;
	pRowData = (int *)celInfo.base + rect.y*w + rect.x ;
	for (i= rect.x; i < rect.x+rect.w; i++){
		pData = pRowData;
		for (j=rect.y; j<rect.y+rect.h; j++){
			*pData++ = 0x77FFFF00;
		}
		pRowData += w;
	}
	
	MV_SHM_CleanAndInvalidateCache(celInfo.uiShmKey, celInfo.size);

	srcRect.w = dstRect.w = w;
	srcRect.h = dstRect.h = h;
	//ret = MV_PE_GFX_DispImg_Resize(hGFX, celInfo.hImage, &srcRect, &dstRect,  &dstRect, plane_id, NULL, NULL);
	ret = MV_PE_GFX_DispImg(hGFX, celInfo.hImage, &srcRect,  &dstRect, plane_id, NULL, NULL);
	if (ret){
	    printf("display image error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
      return ret;
	}

  usleep(10000000);
	MV_PE_GFX_DestroyObject(hGFX,hCel);
	MV_PE_GFX_Close(hGFX);

        return ret;
}

static int draw_full_screen_rawdata()
{
	unsigned int plane_id;
	int ret=0, w=1872, h=1048, i,j;
	FILE *pFile=NULL;
	GFX_RECT	srcRect={0,0,0,0};
	GFX_RECT	dstRect={0,0,0,0};
	MV_PE_GFX_SIZE	size;
	MV_PE_GFX_CelInfo celInfo;

	printf("Please input plane id:\n");
	scanf("%u", &plane_id);
	ret = MV_PE_GFX_Open(hPE, plane_id, "LAYER2", &hGFX);
	if (ret ||(!hGFX)){
	    printf("open gfx error, ret=0x%x\n",ret);
            return ret;
	}

	size = MAKE_SIZE(w,h);
  ret = MV_PE_GFX_CreateCel(hGFX, NULL, MV_PE_GFX_PIX_A8R8G8B8, size, &hCel);
	if (ret ||(!hCel)){
	    printf("create cel error, ret=0x%x\n",ret);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}

	ret = MV_PE_GFX_GetCelInfo(hGFX, hCel, &celInfo);
	if (ret){
	    printf("get celinfo error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	
	pFile = fopen("/data/lhwei1/fb13.rgb", "r");
	if (!pFile){
		printf("can not open /data/lhwei1/fb13.rgb\n");
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
            return ret;
	}
	
	fread(celInfo.base, w*h*4, 1, pFile);
	
	MV_SHM_CleanAndInvalidateCache(celInfo.uiShmKey, w*h*4);

	srcRect.w = dstRect.w = w;
	srcRect.h = dstRect.h = h;
	ret = MV_PE_GFX_DispImg_Resize(hGFX, celInfo.hImage, &srcRect, &dstRect,  &dstRect, plane_id, NULL, NULL);
	//ret = MV_PE_GFX_DispImg(hGFX, celInfo.hImage, &srcRect,  &dstRect, plane_id, NULL, NULL);
	if (ret){
	    printf("display image error, ret=0x%x\n",ret);
	    MV_PE_GFX_DestroyObject(hGFX,hCel);
	    MV_PE_GFX_Close(hGFX);
	    hGFX=NULL;
      return ret;
	}

  usleep(10000000);
	MV_PE_GFX_DestroyObject(hGFX,hCel);
	MV_PE_GFX_Close(hGFX);

        return ret;
}

int gfx_2d_unit_test()
{
	unsigned int plane_id;
        int ret=0;
        GFX_RECT        srcRect={0,0,0,0};
        GFX_RECT        dstRect={0,0,0,0};
        MV_PE_GFX_SIZE  size;
        MV_PE_GFX_CelInfo celInfo;

        printf("Please input plane id:\n");
        scanf("%u", &plane_id);
        ret = MV_PE_GFX_Open(hPE, plane_id, "LAYER2", &hGFX);
        if (ret ||(!hGFX)){
            printf("open gfx error, ret=0x%x\n",ret);
            return ret;
        }

        size = MAKE_SIZE(1920,1080);
        ret = MV_PE_GFX_CreateCel(hGFX, NULL, MV_PE_GFX_PIX_A8R8G8B8, size, &hCel);
        if (ret ||(!hCel)){
            printf("create cel error, ret=0x%x\n",ret);
            MV_PE_GFX_Close(hGFX);
            hGFX=NULL;
            return ret;
        }

        ret=MV_PE_GFX_FillCel(hGFX, hCel, 0xFFFFFFFF, NULL, NULL, NULL);
        if (ret ||(!hCel)){
            printf("fill cel error, ret=0x%x\n",ret);
            MV_PE_GFX_DestroyObject(hGFX,hCel);
            MV_PE_GFX_Close(hGFX);
            hGFX=NULL;
            return ret;
        }

        ret = MV_PE_GFX_GetCelInfo(hGFX, hCel, &celInfo);
        if (ret){
            printf("get celinfo error, ret=0x%x\n",ret);
            MV_PE_GFX_DestroyObject(hGFX,hCel);
            MV_PE_GFX_Close(hGFX);
            hGFX=NULL;
            return ret;
        }

	usleep(1000000);


	{
		char name[64];
		unsigned int offset;
		//Now, share memory is cachable, we must clean the cache first
		//MV_CC_SHM_CleanDCacheRegion((void*)hnd->base, hnd->size);
		memset(name, 0, sizeof(name));
		sprintf(name, "/system/bin/gfx2d_white.rgb");
		FILE *pFile = fopen(name,"w");
		if (pFile){
			printf("dump image to %s, base=0x%x, size=%d\n", 
				name,celInfo.base, celInfo.size);
			fwrite((const void *)celInfo.base, celInfo.size, 1, pFile);
			fclose(pFile);
			offset = MV_SHM_RevertCacheVirtAddr(celInfo.base);
			if (ERROR_SHM_MALLOC_FAILED != offset)
				MV_SHM_CleanCache(offset, celInfo.size);
		}
		else{
			printf("dump file %s failed\n", name);
		}
	}

        ret=MV_PE_GFX_FillCel(hGFX, hCel, 0xFF00FF00, NULL, NULL, NULL);
        if (ret ||(!hCel)){
            printf("fill cel error2, ret=0x%x\n",ret);
            MV_PE_GFX_Close(hGFX);
            hGFX=NULL;
            return ret;
        }

	
	usleep(1000000);
        {
                char name[64];
                //Now, share memory is cachable, we must clean the cache first
                //MV_CC_SHM_CleanDCacheRegion((void*)hnd->base, hnd->size);
                memset(name, 0, sizeof(name));
                sprintf(name, "/system/bin/gfx2d_green.rgb");
                FILE *pFile = fopen(name,"w");
                if (pFile){
                        printf("dump image to %s, base=0x%x, size=%d\n",
                                name,celInfo.base, celInfo.size);
                        fwrite((const void *)celInfo.base, celInfo.size, 1, pFile);
                        fclose(pFile);
                }
                else{
                        printf("dump file %s failed\n", name);
                }
        }

        MV_PE_GFX_DestroyObject(hGFX,hCel);
        MV_PE_GFX_Close(hGFX);

        return ret;
}

int get_plane_bk_color()
{
        int ret = 0;
        int plane_id=1;
        int format, color;

        printf("Please input plane id:\n");
        scanf("%d", &plane_id);

        ret = MV_PE_VideoGetBkColor(hPE, plane_id, &format, &color);
        if (ret){
                printf("MV_PE_VideoGetBkColor faild, ret=0x%x\n", ret);
                return ret;
        }
        printf("plane_id=%d, format=%d, color=0x%x\n",plane_id, format, color);
        return 0;

}

int set_plane_bk_color()
{
        int ret = 0;
        int plane_id=1;
        int r,g,b,color, format;

        printf("Please input plane id:\n");
        scanf("%d", &plane_id);
        printf("Please input the background color\n");
        printf("red\n");
        scanf("%d", &r);
        printf("green\n");
        scanf("%d", &g);
        printf("blue\n");
        scanf("%d", &b);
        color = ((b&0xff)<<16)|((g&0xff)<<8)|((r&0xff));
        printf("color is 0x%x\n", color);

        ret = MV_PE_VideoSetBkColor(hPE, plane_id, MV_PE_VIDEO_COLOR_FORMAT_RGB888, color);
        if (ret){
                printf("MV_PE_VideoSetBkColor faild, ret=0x%x\n", ret);
                return ret;
        }
  ret = MV_PE_VideoGetBkColor(hPE, plane_id, &format, &color);
        if (ret){
                printf("MV_PE_VideoGetBkColor faild, ret=0x%x\n", ret);
                return ret;
        }

        printf("get the value, plane_id=%d, format=%d, color=0x%x\n",plane_id, format, color);
        return 0;

}

int get_cpcb_bk_color()
{
        int ret = 0;
        int cpcb_id=0;
        int format, color;


        ret = MV_PE_VOutGetCPCBBkColor(hPE, cpcb_id, &format, &color);
        if (ret){
                printf("MV_PE_VOutGetCPCBBkColor faild, ret=0x%x\n", ret);
                return ret;
        }
        printf("cpcb_id=%d, format=%d, color=0x%x\n",cpcb_id, format, color);
        return 0;

}

int set_cpcb_bk_color()
{
        int ret = 0;
        int cpcb_id=0;
        int r,g,b,color,format;

        printf("Please input the background color\n");
        printf("red\n");
        scanf("%d", &r);
        printf("green\n");
        scanf("%d", &g);
        printf("blue\n");
        scanf("%d", &b);
        color = ((b&0xff)<<16)|((g&0xff)<<8)|((r&0xff));
        printf("color is 0x%x\n", color);

        ret = MV_PE_VOutSetCPCBBkColor(hPE, cpcb_id, MV_PE_VIDEO_COLOR_FORMAT_RGB888, color);
        if (ret){
                printf("MV_PE_VOutSetCPCBBkColor faild, ret=0x%x\n", ret);
                return ret;
        }
  ret = MV_PE_VOutGetCPCBBkColor(hPE, cpcb_id, &format, &color);
        if (ret){
                printf("MV_PE_VOutGetCPCBBkColor faild, ret=0x%x\n", ret);
                return ret;
        }

        printf("get the value, cpcb_id=%d, format=%d, color=0x%x\n",cpcb_id, format, color);
        return 0;

}

int get_output_resolution()
{
	int ret = 0;
	INT16  outW, outH;
	UINT8 Format;
	MV_PE_VOUT_TG_PARAMS Params;
	ret = MV_PE_VOutGetResolution(hPE, 0, &Format, &Params);
        if (ret){
                printf("MV_PE_VOutGetResolution failed, ret=0x%x\n", ret);
                return ret;
        }
	switch (Format)
            {
                case MV_PE_VOUT_FORMAT_640_480_P_60:
                    outW = 640;
                    outH = 480;
                    break;

                case MV_PE_VOUT_FORMAT_NTSC_M:
                case MV_PE_VOUT_FORMAT_NTSC_J:
                case MV_PE_VOUT_FORMAT_720_480_I_60:
                case MV_PE_VOUT_FORMAT_720_480_I_5994:
                case MV_PE_VOUT_FORMAT_720_480_P_60:
                case MV_PE_VOUT_FORMAT_720_480_P_5994:
                    outW = 720;
                    outH = 480;
                    break;

                case MV_PE_VOUT_FORMAT_PAL_M:
                case MV_PE_VOUT_FORMAT_PAL_BGH:
                case MV_PE_VOUT_FORMAT_720_576_I_50:
                case MV_PE_VOUT_FORMAT_720_576_P_50:
                    outW = 720;
                    outH = 576;
                    break;

                case MV_PE_VOUT_FORMAT_1280_720_P_30:
                case MV_PE_VOUT_FORMAT_1280_720_P_2997:
                case MV_PE_VOUT_FORMAT_1280_720_P_25:
                case MV_PE_VOUT_FORMAT_1280_720_P_60:
                case MV_PE_VOUT_FORMAT_1280_720_P_5994:
                case MV_PE_VOUT_FORMAT_1280_720_P_50:
                    outW = 1280;
                    outH = 720;
                    break;

                case MV_PE_VOUT_FORMAT_1920_1080_I_60:
                case MV_PE_VOUT_FORMAT_1920_1080_I_5994:
                case MV_PE_VOUT_FORMAT_1920_1080_I_50:
                case MV_PE_VOUT_FORMAT_1920_1080_P_30:
                case MV_PE_VOUT_FORMAT_1920_1080_P_2997:
                case MV_PE_VOUT_FORMAT_1920_1080_P_25:
                case MV_PE_VOUT_FORMAT_1920_1080_P_24:
                case MV_PE_VOUT_FORMAT_1920_1080_P_2398:
                case MV_PE_VOUT_FORMAT_1920_1080_P_60:
                case MV_PE_VOUT_FORMAT_1920_1080_P_5994:
                case MV_PE_VOUT_FORMAT_1920_1080_P_50:
                default:
                    outW = 1920;
                    outH = 1080;
                    break;
            }

	printf("output resolution=%d, width=%d, height=%d\n", Format,outW, outH);
	return 0;
}

static void ShowHelp()
{
     printf("  Berlin SetPlane Control\n\n");
    // printf("  Version: %s\n", version);
       // printf("  Author: \n");
    printf("  Usage: 1. setplane command \n\n");
    printf("         2. setplane; \n             command \n\n");
    
    printf("  All supported commands:\n\n");
    printf("         %-6d- %s\n", 0, "get plane information");
    printf("         %-6d- %s\n", 1, "plane visible setting");
    printf("         %-6d- %s\n", 2,"2d gfx drawing+displaying");
    printf("         %-6d- %s\n", 3 ,"swap zorder");
    printf("         %-6d- %s\n", 4,"set scaling");
    printf("         %-6d- %s\n", 5,"get scaling");
    printf("         %-6d- %s\n",6,"pe suspend");
    printf("         %-6d- %s\n",7,"pe resume");
    printf("         %-6d- %s\n",8,"2d gfx unit test");
    printf("         %-6d- %s\n",11,"get plane background color");
    printf("         %-6d- %s\n",12,"set plane background color");
    printf("         %-6d- %s\n",13,"get cpcb background color");
    printf("         %-6d- %s\n",14,"set cpcb background color");
    printf("         %-6d- %s\n",15,"draw a null image");
    printf("         %-6d- %s\n",16,"draw a w*h image with gpu2d");
    printf("         %-6d- %s\n",17,"draw a w*h with no gpu");
    printf("         %-6d- %s\n",18,"draw a full screen rawdata");
    printf("         %-6d- %s\n",19,"get output resolution");
    printf("         %-6s- %s\n","help","list all supported commands");
    printf("         %-6s- %s\n","quit","exiting\n");


}



static int ExecuteCommand(char *key)
{
    int	  ret = 0;
    int choice=atoi(key); 
    
    if(choice == 0)
    {
        if (strcmp(key,"0") != 0)
            choice = 40;     //invalid arg
     }

        switch(choice){
                  case 0:
		 	ret=get_plane_information();
		 	break;
		case 1:
		  	ret = set_plane_visible();
			break;
	  	case 2:
		 	ret = draw_2d_graphics();
		 	break;
	  	case 3:
		 	ret = swap_zorder();
		 	break;
		case 4:
			ret = set_scaling();
			break;
	  	case 5:
		 	ret = get_scaling();
		  	break;
	  	case 6:
		 	ret = pe_suspend();
		  	break;
	  	case 7:
		 	ret = pe_resume();
		  	break;
	  	case 8:
		 	ret = gfx_2d_unit_test();
		  	break;
	  	case 11:
		 	ret = get_plane_bk_color();
		  	break;
	  	case 12:
			ret = set_plane_bk_color();
		  	break;
	  	case 13:
		 	ret = get_cpcb_bk_color();
		  	break;
		case 14:
	  		ret = set_cpcb_bk_color();
		  	break;
	  	case 15:
		  	ret = draw_null_image();
		  	break;
	  	case 16:
		  	ret = draw_w_h_by_gpu2d();
		  	break;
	  	case 17:
		  	ret = draw_w_h_no_gpu();
		  	break;
	  	case 18:
		  	ret = draw_full_screen_rawdata();
		  	break;
	  	case 19:
		  	ret = get_output_resolution();
		  	break;
	  	case 30:
	  		ShowHelp();
			break;
	  	default:
		 	printf("\n	Invild option\n");
		 	break;
	  }

	  return ret;

}


static BOOL ParseCommand(char *pCmd)
{
    char *pArg[MAX_ARG_CNT];
    int argc = 0;
    //BOOL bValid = FALSE;
    int i;

    while (*pCmd == 0x20) pCmd++;
    
    if (strlen(pCmd) == 0)
        return TRUE; 
	
    if (strcmp(pCmd, "quit") == 0)
    {	
        return FALSE;
    }

    if (strcmp(pCmd, "help") == 0)
    {
        pCmd = "30";
    }

    pArg[argc] = pCmd;
    argc++;
    
    while(*pCmd != 0)
    {
        while ((*pCmd != 0x20) && (*pCmd != 0))
        {
            pCmd++;
        }

        if (*pCmd == 0x20)
        {
            *pCmd++ = 0;
            while (*pCmd == 0x20) pCmd++;
            if (*pCmd != 0)
            {
                pArg[argc++] = pCmd;
            }
        }
    }


    if(argc != 1)
    {

        printf("\n	Invalid option\n");
        return TRUE;
    }
	
    ExecuteCommand(pArg[0]);

  //  pArg[argc] = pCmd;
  //  argc++;


/*
    if (!bValid)
    {
        printf("Invalid command! Type help to see all supported command!\n");
    }
*/
    /*for (i = 0; i < argc; i++)
    {
        printf("arg%d = %s\n", i, pArg[i]);
    }*/

    return TRUE;
}

static int GetCommand(char *pCmd, int iMaxSize)
{
    int count;
    char ch;

    if ((pCmd == 0) || (iMaxSize == 0))
    {
        return 0;
    }

    count = 0;
    iMaxSize--;
    while((ch = getchar()) != 0x0a)
    {
        if (count < iMaxSize)
        {
            pCmd[count++] = ch;
        }
    }

    pCmd[count] = '\0';

    return count;
}



int SetPlaneMain(int argc, char *argv[])
{

	  int	  ret = 0;
	
	  ret = MV_OSAL_Init();
	  if (ret){
		  printf("MV_OSAL_INIT faild, ret=0x%x\n", ret);
		  return ret;
	  }
	  printf("MV_OSAL_Init success!\n");
	
	  ret = MV_PE_Init(&hPE);
	  if (ret){
		   printf("PE faild, ret=0x%x\n", ret);
			   return ret;
	  }
	  printf("MV_PE_Init success!\n");
	
	
	
	if(argc == 1)
	{	
    		//HRESULT rc = 0;
   		char    Cmd[MAX_CMD_LEN];
    		int len;

    		printf("***********************************************\n");
    		printf("*      Set Plane  *\n");
    		printf("***********************************************\n\n");

   	 	do
    		{
        			printf(SetPlane_PROMPT);
        			len = GetCommand(Cmd, MAX_CMD_LEN);
        			//printf("Command = %s, len = %d\n", Cmd, len);
    		}
   	 	while (ParseCommand(Cmd));
	}


	else if(argc == 2)
		{
			if(strcmp(argv[1],"help") == 0)
			{
				argv[1] = "30";
			}

			if (strcmp(argv[1],"quit") == 0)
				goto __exit;
			
			ret = ExecuteCommand(argv[1]);		
		}

	else
		{
			printf("\n	Invild option\n");
			ret = -1;
		}

__exit:

    MV_PE_Remove(hPE);
    hPE = NULL;
    MV_OSAL_Exit();

    printf("exit!\n");
    return ret;
}
