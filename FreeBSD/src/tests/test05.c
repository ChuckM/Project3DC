#include <stdio.h>
#include <stdlib.h>
#include <p3dc.h>

p3dc_PNT3 m[4] = {
    { -4.0, -4.0, -4.0 },
	{  4.0, -4.0, -4.0 },
    {  4.0,  4.0,  4.0 },
    { -4.0,  4.0,  4.0 },
};

void
main(int argc, char *argv[]) {
	p3dc_CMRA *cam1 = p3dc_new_camera(60.0f, 0.0f, 0.0f, -40.0f);
	p3dc_CMRA *cam2 = p3dc_new_camera(90.0f, 0.0f, 0.0f, -50.0f);
	p3dc_POLY *poly = p3dc_new_polygon(&COLOR_green, 4, m);
	p3dc_NODE	*n = NULL;
	p3dc_VRTX	*tV;
	int			i;
	float		tZ;

	if (argc == 2) {
		int f = atoi(argv[1]);
		p3dc_fov_camera(cam1, (float) f);
	}

	p3dc_frame_camera(cam1, 160, 120, 480, 360);  
/*	p3dc_setd_camera(cam2, cam1->d*2); */
	printf("Camera 1 d = %f, fov = %f\n", 
					cam1->d, cam1->fov);
	printf("Camera 2 d = %f, fov = %f\n", 
					cam2->d, cam2->fov);
	printf("Camera1 Transforms :\n");
	for (n = poly->pn.head, i = 0; n; n = n->nxt, i++) {
		int sx, sy;
		p3dc_VRTX *vv = (p3dc_VRTX *)(n->data.p);
		p3dc_xfrm_vrtx(NULL, &(cam1->view_xfrm), vv);
		printf("[%d] - Orig (%3.2f, %3.2f, %3.2f) ==> [XID = %d] (%f, %f, %f, %f)\n",
				i, vv->v.x, vv->v.y, vv->v.z, vv->xid,
				   vv->tv.x, vv->tv.y, vv->tv.z, vv->tv.w);
		sx = vv->tv.x/vv->tv.w * cam1->vp.scale_x; /*  + cam1->scx; */
		sy = vv->tv.y/vv->tv.w * cam1->vp.scale_y; /*  + cam1->scy; */
		printf("	----> %d, %d\n", sx, sy);
	}
		
	printf("Camera 2 Transforms :\n");
	for (n = poly->pn.head, i = 0; n; n = n->nxt, i++) {
		int sx, sy;
		p3dc_VRTX *vv = (p3dc_VRTX *)(n->data.p);
		p3dc_xfrm_vrtx(NULL, &(cam2->view_xfrm), vv);
		printf("[%d] - Orig (%3.2f, %3.2f, %3.2f) ==> [XID = %d] (%f, %f, %f, %f)\n",
				i, vv->v.x, vv->v.y, vv->v.z, vv->xid,
				   vv->tv.x, vv->tv.y, vv->tv.z, vv->tv.w);
		sx = vv->tv.x/vv->tv.w * cam2->vp.scale_x; /*  + cam2->scx; */
		sy = vv->tv.y/vv->tv.w * cam2->vp.scale_y; /*  + cam2->scy; */
		printf("	----> %d, %d\n", sx, sy);
	}
	for (tZ = -30; tZ < 30; tZ += 3.0f) {
		p3dc_PNT2 tt[2];

		tV = p3dc_new_vrtx(10, 10, tZ);
		p3dc_xfrm_vrtx(NULL, &(cam1->view_xfrm), tV);
		tt[0].u = tV->tv.x/tV->tv.w * cam1->vp.scale_x;
		tt[0].v = tV->tv.y/tV->tv.w * cam1->vp.scale_y;
#if 0
		printf("%5.3f : cam1 X, Y -> %7.4f, %7.4f\n", tZ, tt[0].u, tt[0].v);
#endif
		p3dc_xfrm_vrtx(NULL, &(cam2->view_xfrm), tV);
		tt[1].u = tV->tv.x/tV->tv.w * cam2->vp.scale_x;
		tt[1].v = tV->tv.y/tV->tv.w * cam2->vp.scale_y;
#if 0
		printf("      : cam2 X, Y -> %7.4f, %7.4f\n", tt[1].u, tt[1].v);
#endif
		printf("%5.3f :              %7.4f, %7.4f\n", tZ, tt[0].u - tt[1].u,
								tt[0].v - tt[1].v);
	}
		
}
	
	
