/* Pract2  RAP 09/10    Javier Ayllon*/

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h> 
#include <assert.h>   
#include <unistd.h>   
#define NIL (0)       
#define N_NODOS 10
#define LONGITUD_FICHERO 400
#define NORMAL 0
#define SEPIA 1 /*Rx0.439 Gx0.259 Bx0.078*/
#define LOW_COMSTRAST 2 /*Rx0.333 Gx0.333 Bx0.333*/
/*Variables Globales */

XColor colorX;
Colormap mapacolor;
char cadenaColor[]="#000000";
Display *dpy;
Window w;
GC gc;

/*Funciones auxiliares */

void initX() {

      dpy = XOpenDisplay(NIL);
      assert(dpy);

      int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
      int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

      w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
                                     400, 400, 0, blackColor, blackColor);
      XSelectInput(dpy, w, StructureNotifyMask);
      XMapWindow(dpy, w);
      gc = XCreateGC(dpy, w, 0, NIL);
      XSetForeground(dpy, gc, whiteColor);
      for(;;) {
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == MapNotify)
                  break;
      }


      mapacolor = DefaultColormap(dpy, 0);

}

void dibujaPunto(int x,int y, int r, int g, int b) {

        sprintf(cadenaColor,"#%.2X%.2X%.2X",r,g,b);
        XParseColor(dpy, mapacolor, cadenaColor, &colorX);
        XAllocColor(dpy, mapacolor, &colorX);
        XSetForeground(dpy, gc, colorX.pixel);
        XDrawPoint(dpy, w, gc,x,y);
        XFlush(dpy);

}



/* Programa principal */

int main (int argc, char *argv[]) {

  int rank,size;
  MPI_Comm commPadre;
  int tag;
  MPI_Status status;
  int errcodes[N_NODOS];
  int buff_punto[5]; /*Indicamos en 0->i, 1->j, 2->R, 3->G, 4->B*/

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_get_parent( &commPadre );
  if ((commPadre==MPI_COMM_NULL) && (rank==0)){

	initX();
      MPI_Comm_spawn("pract2", MPI_ARGV_NULL, N_NODOS, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &commPadre,errcodes);
      
      for (int i = 0;i<LONGITUD_FICHERO*LONGITUD_FICHERO;i++){
            MPI_Recv(&buff_punto,5, MPI_INT, MPI_ANY_SOURCE,0,commPadre,&status); /*En este caso el proceso padre recibe los datos del intercomunicador que lo hijos han enviado el mensaje*/
            
            dibujaPunto(buff_punto[0],buff_punto[1],buff_punto[2],buff_punto[3],buff_punto[4]);
      }
	/* Codigo del maestro */

	/*En algun momento dibujamos puntos en la ventana algo como
	dibujaPunto(x,y,r,g,b);  */
      sleep(1);
        

  }else {
    /* Codigo de todos los trabajadores */
    /* El archivo sobre el que debemos trabajar es foto.dat */
      MPI_File fd;
      MPI_Status status;
      /*Dividimos el tamaño del fichero en los distintos ranks*/
      int fila_fichero = LONGITUD_FICHERO / N_NODOS;
      int inicio_fichero = rank * fila_fichero;
      int final_fichero = ((rank+1) * fila_fichero)-1;
      if(rank == N_NODOS-1){
            final_fichero = LONGITUD_FICHERO-1;
      }

      MPI_Offset area_rank = fila_fichero * LONGITUD_FICHERO * 3 * sizeof(unsigned char);

      unsigned char colores[3]; /*Aquí tenemos el array de los colores*/
      MPI_File_open(MPI_COMM_WORLD, "foto.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &fd);/*Abrimos el fichero de solo lectura*/


      MPI_File_set_view(fd,area_rank*rank,MPI_UNSIGNED_CHAR,MPI_UNSIGNED_CHAR,"native",MPI_INFO_NULL);
      
      printf("Hola soy %d: PRINCIPIO: %d Y FIN: %d\n",rank,inicio_fichero,final_fichero);

      for(int i = inicio_fichero; i<=final_fichero;i++){
            for(int j = 0;j<LONGITUD_FICHERO;j++){

                buff_punto[0] = i;
                buff_punto[1] = j;

                MPI_File_read(fd, colores, 3 , MPI_UNSIGNED_CHAR, &status);
                buff_punto[2] = (int)colores[0]*0.47;
                buff_punto[3] = (int)colores[1]*0.47;
                buff_punto[4] = (int)colores[2]*0.47;

                MPI_Bsend(buff_punto, 5, MPI_INT , 0, 0, commPadre);
            }
      }

      MPI_File_close(&fd);

  }

  MPI_Finalize();

}

