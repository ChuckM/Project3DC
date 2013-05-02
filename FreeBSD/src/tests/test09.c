#include <stdio.h>
#include <p3dc.h>

p3dc_PNT3	pnts[10] = {
	{5, 5, 5},
	{-5, 5, 5},
	{-5, -5, 5},
	{5, -5, 5},
	{5, 5, -5},
	{-5, 5, -5},
	{-5, -5, -5},
	{5, -5, -5},
	{ 0, 0, 0},
};

p3dc_POINT *P[10];

p3dc_LINE *L[12];

p3dc_LINE *A[3]; /* Axes line */

void
main(int argc, char *argv[]) {
	int i;
	p3dc_CMRA *cam = p3dc_new_camera(35, 10, 10, -30);
	
	p3dc_open();
	p3dc_log_open("/dev/tty");
	p3dc_draw_axes(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, 5.0);
	p3dc_log("Preparing to render 8 spots\n");
	i = 0;
	while (i < 8) {
		P[i] = p3dc_new_point(&COLOR_green, &(pnts[i]));
		i++;
	}
	L[0] = p3dc_new_line(&COLOR_green, &(pnts[0]), &(pnts[1]));
	L[1] = p3dc_new_line(&COLOR_green, &(pnts[1]), &(pnts[2]));
	L[2] = p3dc_new_line(&COLOR_green, &(pnts[2]), &(pnts[3]));
	L[3] = p3dc_new_line(&COLOR_green, &(pnts[3]), &(pnts[0]));
	L[4] = p3dc_new_line(&COLOR_green, &(pnts[4]), &(pnts[5]));
	L[5] = p3dc_new_line(&COLOR_green, &(pnts[5]), &(pnts[6]));
	L[6] = p3dc_new_line(&COLOR_green, &(pnts[6]), &(pnts[7]));
	L[7] = p3dc_new_line(&COLOR_green, &(pnts[7]), &(pnts[4]));

	L[8] = p3dc_new_line(&COLOR_red, &(pnts[0]), &(pnts[4]));
	L[9] = p3dc_new_line(&COLOR_red, &(pnts[1]), &(pnts[5]));
	L[10] = p3dc_new_line(&COLOR_red, &(pnts[2]), &(pnts[6]));
	L[11] = p3dc_new_line(&COLOR_red, &(pnts[3]), &(pnts[7]));
	
	p3dc_prepare_frame(&COLOR_black);
#define DRAW_LINES
#ifdef DRAW_LINES
	for (i = 0; i < 12; i++) {
		p3dc_log("Draw line #%d\n", i+1);
		p3dc_draw_line(&(cam->vp), &(cam->view_xfrm), 0, L[i]);
	}
#else
	for (i = 0; i < 8; i++) {
		p3dc_log("Draw spot #%d\n", i+1);
		p3dc_draw_point(&(cam->vp), &(cam->view_xfrm), 
				P3DC_DRAW_NOSHADE, P[i]);
	}
#endif
	p3dc_flip_frame(1);
	p3dc_snapshot("test09.png", "Chuck McManis", NULL);
	p3dc_close();
	exit(0);
}
