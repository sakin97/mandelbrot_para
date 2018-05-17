/*
 * mandelbrot.c
 * For Parrallel and Distributed Programming
 * Based on Mandelbortset.c by Shibin K.Reeny.
 * Copyright Yasuhiro Ohtaki
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the
 * GNU General Public License as published by
 * the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

/* Mandelbrotset.c
 * Copyright Shibin K.Reeny
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the
 * GNU General Public License as published by
 * the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <GL/gl.h>
#include <GL/glut.h>
#include <pthread.h>

#include <stdio.h>

#define IMAGE_X 1440
#define IMAGE_Y 840

#define MAX_ITERATION 10000

/*
 *defining a RGB struct to color the pixel
 */
struct Type_rgb{
  float r;
  float g;
  float b;
};

struct point {
        int xstart, xend;   //描画する部分のX軸方向の始点，終点
        int ystart, yend;   //描画する部分のY軸方向の始点，終点
};

/*
 * 画像中の各ピクセルの色を保持する配列
 */

struct Type_rgb pixels[IMAGE_Y * IMAGE_X];

/*
 * 収束にかかった回数に応じて、ピクセルの色を決める関数
 */
struct Type_rgb setMyColor( int iteration )
{
    struct Type_rgb col;
    if(iteration >= MAX_ITERATION){
        /*
         * 収束しなかったら黒
         */
        col.r =col.g = col.b = 0;
    }else{
        /*
         * setting color pixel to the reset of the coordinate by the
         * number of iteration before bailout.
         */
#define PPSTEP 10
        col.b = (iteration       % PPSTEP) / (float)PPSTEP ;
        col.g = ((iteration /2 ) % PPSTEP) / (float)PPSTEP;
        col.r = ((iteration /4 ) % PPSTEP) / (float)PPSTEP;
    }
    return col;
}

/*
 * 複素平面上のある一点における収束回数を求める mandelbrotsetの下請け関数。
 * （複素平面上の点(r0,i0)における収束回数）
 * この関数は変更する必要なし。
 */
int mandel_sub(float r0,float i0)
{
    /*
     * r0 :- is the real part
     * i0 :- is the imaginary
     * r and i :- is the real and imaginary part of Zn.
     * iteration :- is to keep control variable of the number of iteration
     * MAX_ITERATION :- maximum number of iteration
     *        (which is one of bailout condition)
     */

    float r, i, rtemp;
    int iteration;

    r = 0;
    i = 0;
    iteration = 0;
    /*
     * (r*r) + (i*i) < (2*2) is the 2nd bailout condition
     * i.e. the mandelbrot set is always within a radius of 2.
     */
    while(((r*r) + (i*i) < (2*2)) && iteration < MAX_ITERATION){
        rtemp = (r*r) - (i*i) + r0;
        i = (2*r*i) + i0;
        r = rtemp;
        iteration = iteration + 1;
    }
    return iteration;
}

/*
 * 複素平面（実部 -2.5 から 1.1, 虚部 -1.0 から 1.1の範囲）における
 * マンデルブロ集合の図全体を計算する関数。
 * この関数を書き換える。あるいは、置き換える関数を作成する。
 */
void *mandelbrotset(void *datas)
{
    /*
     * xx, yy はwindow上の座標。
     * r0,i0 はそれを上記の複素平面にマップした座標。
     */
    float xx,yy,r0, i0;
    int iteration;
    long loc;

    struct point *p;
    p = (struct point *)datas;

    //画面上の座標に対して
    for(yy = p->ystart; yy < p->yend; yy++){
        for(xx = p->xstart; xx < p->xend; xx++){

            //複素平面上の座標に変換
            i0= 2.1 * yy / IMAGE_Y -1.0;
            r0= 3.6 * xx / IMAGE_X -2.5;

            //収束にかかる回数を計算
            iteration=mandel_sub(r0,i0);

            //画像を保持する配列に色情報を格納。
            loc = yy*IMAGE_X + xx;
            pixels[loc]= setMyColor(iteration);
        }
    }
    pthread_exit(NULL);
}

/*
 * ----------------------------------------------------------------------------
 */
void idle(void)
{
    glutPostRedisplay();
}

void Init( )
{
    glViewport(0, 0, IMAGE_X, IMAGE_Y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity( );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity( );
    gluOrtho2D(0, IMAGE_X, 0, IMAGE_Y);

    int i;
    /*
     * Initializing all the pixels to white.
     */
    for(i = 0; i < IMAGE_X * IMAGE_Y; i++){
        pixels[i].r = 1;
        pixels[i].g = 1;
        pixels[i].b = 1;
    }
}

void onDisplay()
{
    /*
     * pixels配列の中身を描画
     */
    glDrawPixels(IMAGE_X, IMAGE_Y, GL_RGB, GL_FLOAT, pixels);
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':
        case 'Q':
            glutIdleFunc(NULL);
            exit(0);
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    pthread_t threads[2][2];

    struct point points[2][2];

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowPosition (100, 100);
    glutInitWindowSize (IMAGE_X, IMAGE_Y);
    glutCreateWindow ("Mandelbrot");

    Init();

    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glutDisplayFunc(onDisplay);
    glutKeyboardFunc(keyboard);

    glutIdleFunc(idle);


    //描画領域を4つに分け，
    //それぞれの領域でmandelbrotsetを実行するスレッドを生成する
    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            points[i][j].xstart = j*IMAGE_X/2;
            points[i][j].xend = points[i][j].xstart + IMAGE_X/2;
            points[i][j].ystart = i*IMAGE_Y/2;
            points[i][j].yend = points[i][j].ystart + IMAGE_Y/2;

            pthread_create(&threads[i][j], NULL, mandelbrotset, &points[i][j]);
        }
    }

    /* GLの描画ルーチンで計算中の配列を随時 描画する */
    glutMainLoop();

    //スレッドの終了を待つ
    for (i = 0; i < 2; i++) { 
        for (j = 0; j < 2; j++) {
            pthread_join(threads[i][j], NULL);
        }
    }

    return EXIT_SUCCESS;
}

