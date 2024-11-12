/***************************************************
Víctor Pascual Muñoz 4CO21 :)
***************************************************/
#define PROYECTO "SIMULADOR ESPACIAL"
#include <iostream> // Biblioteca de entrada salida
#include <cmath> // Biblioteca matematica de C
#include <gl\freeglut.h> // Biblioteca grafica
#include <codebase.h>
#include <freeimage/FreeImage.h> // Biblioteca de gestion de imagenes
#include <irrKlang.h> // Biblioteca de sonido
#include <vector>
#pragma comment(lib, "irrKlang.lib")

using namespace irrklang;
using namespace std;
using namespace cb;
static const int DIM_ESPACIO = 1000; // Longitud del lado del espacio
//static GLuint ejes; // Identificador de los ejes
//static int xantes, yantes; // Valor del pixel anterior
static float girox = 0, giroy = 0, giroz = 0; // Valor del giro a acumular
//static float escalado = 1; // Valor del escalado acumulado
//static enum Interaccion { GIRO, ESCALADO } accion; // Tipo de acción de inspección



static float velocidad_actual = 0;
static const float max_velocidad = 2;
static float aceleracion_actual = 0.2;
//static float posicionActualCamara[3] = {0,1,1};
static float giroHorizontalCamara = 0;
static float cabeceoVertical = 0;

static float giro_nave = 0;

static bool mostrarNave = true;
static bool lucesEncendidas= true;

static const int tasaFPS = 40; // tasa que se quiere como maximo
static float tiempo_transcurrido = 0;

static int window_width = 1000; //puede cambiar
static int window_height = 1000;


static ISoundEngine* engine;
static ISoundSource* engineExplosion;
static ISoundSource* engineBlaster;

struct proyectil {
	Vec3 posicion;
	Vec3 direccion;
};

vector<proyectil> proyectiles;
static float velocidad_proyectil = 3;

static float velocidad_tiempo = 1;

/*
Ids de texturas, 
0 corresponde al cielo, 1 luna, 2 sol, 3 tierra, 4 jupiter, 
5 nave, 6 pista, 7 Marte, 8 Venus, 9 nave izquierda, 
10 nave derecha, 11 nave abajo,12 nave cubemap, 13 Bala
*/ 
static GLuint texturas[14]; 

static Sistema3d sistema_espacio = Sistema3d(
	Vec3(1, 0, 0),	// u / x
	Vec3(0, 0, 1),	// v / y
	Vec3(0, -1, 0),	// w / z
	Vec3(100, -300, 0)); // origen o posicion inicial
static Sistema3d sistema_nave = sistema_espacio;
static bool bloquear_cabeza = false;
//LUNA
static const float radioLUNA = 10.0; // Radio de giro respecto a la tierra
static float anguloLUNA = 0.0; // Angulo de travelling inicial
static float posicionLUNA[] = { 0,radioLUNA,0 }; // Posicion inicial 
static const float velocidad = 24.0 * 3.1415926 / 180; // radi

//Tierra
static const float radioTIERRA = 150.0; // Radio de giro 
static float anguloTIERRA = std::rand() % 361; // Angulo de travelling inicial
static float posicionTIERRA[] = { 0,0,0 }; // Posicion inicial 
static const float velocidadTIERRA = -6.0 * 3.1415926 / 180; // radi
static float rotacionTierra = 0;
static float velocidadRotacionTIERRA = 30;//grados por segundo


//Jupiter
static const float radioJUPITER = 250.0; // Radio de giro 
static float anguloJUPITER = std::rand() % 361; // Angulo de travelling inicial
static float posicionJUPITER[] = { 0,0,0 }; // Posicion inicial 
static const float velocidadJUPITER = +3.0 * 3.1415926 / 180; // radi
static float rotacionJUPITER = 0;
static float velocidadRotacionJUPITER = 35;//grados por segundo

//Marte
static const float radioMARTE = 200.0; // Radio de giro 
static float anguloMARTE = std::rand() % 361; // Angulo de travelling inicial
static float posicionMARTE[] = { 0,0,0 }; // Posicion inicial 
static const float velocidadMARTE = -3.0 * 3.1415926 / 180; // radi
static float rotacionMARTE = 0;
static float velocidadRotacionMARTE = 30;//grados por segundo

//Venus
static const float radioVENUS = 100.0; // Radio de giro 
static float anguloVENUS = std::rand() % 361; // Angulo de travelling inicial
static float posicionVENUS[] = { 0,0,0 }; // Posicion inicial 
static const float velocidadVENUS = 90.0 * 3.1415926 / 180; // radi
static float rotacionVENUS = 0;
static float velocidadRotacionVENUS = 40;//grados por segundo

static float posicionSOL[] = { 0,0,0 };  


static void cargarImagen(const char* nombre)
// Uso de FreeImage para cargar la imagen en cualquier formato
// nombre: nombre del fichero con extensión en el mismo directorio que el proyecto
// o con su path completo
{
	// Detección del formato, lectura y conversion a BGRA
	FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(nombre, 0);
	FIBITMAP* imagen = FreeImage_Load(formato, nombre);
	FIBITMAP* imagen32b = FreeImage_ConvertTo32Bits(imagen);
	// Lectura de dimensiones y colores
	int w = FreeImage_GetWidth(imagen32b);
	int h = FreeImage_GetHeight(imagen32b);
	GLubyte* texeles = FreeImage_GetBits(imagen32b);
	// Carga como textura actual
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texeles);
	// Liberar recursos
	FreeImage_Unload(imagen);
	FreeImage_Unload(imagen32b);
}
//void loadTexture()
//// Funcion de carga de texturas e inicializacion
//{
//	//1a. Generar un objeto textura
//	glGenTextures(1, &texturas[0]);
//	//1b. Activar el objeto textura
//	glBindTexture(GL_TEXTURE_2D, texturas[0]);
//	//1c. Cargar la imagen que servira de textura
//	cargarImagen("cielo.jpg");
//	//1b. Activar el objeto textura
//	glBindTexture(GL_TEXTURE_2D, texturas[0]);
//	//1c. Cargar la imagen que servira de textura
//	cargarImagen("cielo.jpg");
//	//1d. Habilitar las texturas
//	glEnable(GL_TEXTURE_2D);
//}

void generar_cubemap(int textura) {
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glTranslatef(sistema_espacio.geto().x, sistema_espacio.geto().y, sistema_espacio.geto().z);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBindTexture(GL_TEXTURE_2D, texturas[textura]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glPolygonMode(GL_FRONT, GL_FILL);
	if (textura == 12) {
		Vec3 u = sistema_nave.getu().normalize();
		Vec3 v = sistema_nave.getv().normalize();
		Vec3 w = sistema_nave.getw().normalize();
		Vec3 origen = sistema_nave.geto();

		GLfloat matriz[16] = {
			u.x, u.y, u.z, 0.0f,
			-w.x, -w.y, -w.z, 0.0f,
			v.x, v.y, v.z, 0.0f,
			origen.x, origen.y, origen.z, 1.0f
		};

		glMultMatrixf(matriz);

	}
	

	// parte frontal
	GLfloat frontal_arriba_derecha[] = { DIM_ESPACIO, DIM_ESPACIO, DIM_ESPACIO };
	GLfloat frontal_arriba_izquierda[] = { -DIM_ESPACIO , DIM_ESPACIO, DIM_ESPACIO };
	GLfloat frontal_abajo_izquierda[] = { -DIM_ESPACIO , DIM_ESPACIO, -DIM_ESPACIO };
	GLfloat frontal_abajo_derecha[] = { DIM_ESPACIO, DIM_ESPACIO, -DIM_ESPACIO };

	glPushMatrix();
	glRotatef(180, 0, 1, 0);
	quadtex(frontal_arriba_derecha, frontal_arriba_izquierda, frontal_abajo_izquierda, frontal_abajo_derecha, 0.25, 0.5, 0.33, 0.66, 0, 0);
	glPopMatrix();

	

	// parte atrás
	GLfloat atras_arriba_izquierda[] = { -DIM_ESPACIO, -DIM_ESPACIO, DIM_ESPACIO };
	GLfloat atras_arriba_derecha[] = { DIM_ESPACIO, -DIM_ESPACIO, DIM_ESPACIO };
	GLfloat atras_abajo_derecha[] = { DIM_ESPACIO, -DIM_ESPACIO, -DIM_ESPACIO };
	GLfloat atras_abajo_izquierda[] = { -DIM_ESPACIO, -DIM_ESPACIO, -DIM_ESPACIO };
	glPushMatrix();
	glRotatef(180, 0, 1, 0);
	quadtex(atras_arriba_izquierda, atras_arriba_derecha, atras_abajo_derecha, atras_abajo_izquierda, 0.75, 1, 0.33, 0.66, 0, 0);
	glPopMatrix();

	// parte izquierda
	GLfloat izquierda_arriba_derecha[] = { -DIM_ESPACIO , DIM_ESPACIO, DIM_ESPACIO };
	GLfloat izquierda_arriba_izquierda[] = { -DIM_ESPACIO, -DIM_ESPACIO, DIM_ESPACIO };
	GLfloat izquierda_abajo_izquierda[] = { -DIM_ESPACIO, -DIM_ESPACIO, -DIM_ESPACIO };
	GLfloat izquierda_abajo_derecha[] = { -DIM_ESPACIO , DIM_ESPACIO, -DIM_ESPACIO };
	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	quadtex(izquierda_arriba_derecha, izquierda_arriba_izquierda, izquierda_abajo_izquierda, izquierda_abajo_derecha, 0, 0.25, 0.33, 0.66, 0, 0);
	glPopMatrix();

	// parte derecha
	GLfloat derecha_arriba_izquierda[] = { DIM_ESPACIO, -DIM_ESPACIO, DIM_ESPACIO };
	GLfloat derecha_arriba_derecha[] = { DIM_ESPACIO, DIM_ESPACIO, DIM_ESPACIO };
	GLfloat derecha_abajo_derecha[] = { DIM_ESPACIO, DIM_ESPACIO, -DIM_ESPACIO };
	GLfloat derecha_abajo_izquierda[] = { DIM_ESPACIO, -DIM_ESPACIO, -DIM_ESPACIO };
	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	quadtex(derecha_arriba_izquierda, derecha_arriba_derecha, derecha_abajo_derecha, derecha_abajo_izquierda, 0.5, 0.75, 0.33, 0.66, 0, 0);
	glPopMatrix();

	
	// parte de arriba
	GLfloat arriba_derecha_abajo[] = { DIM_ESPACIO, -DIM_ESPACIO, DIM_ESPACIO };
	GLfloat arriba_izquierda_abajo[] = { -DIM_ESPACIO, -DIM_ESPACIO, DIM_ESPACIO };
	GLfloat arriba_izquierda_arriba[] = { -DIM_ESPACIO , DIM_ESPACIO, DIM_ESPACIO };
	GLfloat arriba_derecha_arriba[] = { DIM_ESPACIO, DIM_ESPACIO, DIM_ESPACIO };
	glPushMatrix();
	glRotatef(180, 0, 0, 1);
	quadtex(arriba_derecha_abajo, arriba_izquierda_abajo, arriba_izquierda_arriba, arriba_derecha_arriba, 0.25, 0.5, 0.66, 1, 0, 0);
	glPopMatrix();

	// parte de abajo
	GLfloat abajo_derecha_arriba[] = { DIM_ESPACIO, DIM_ESPACIO, -DIM_ESPACIO };
	GLfloat abajo_izquierda_arriba[] = { -DIM_ESPACIO , DIM_ESPACIO, -DIM_ESPACIO };
	GLfloat abajo_izquierda_abajo[] = { -DIM_ESPACIO, -DIM_ESPACIO, -DIM_ESPACIO };
	GLfloat abajo_derecha_abajo[] = { DIM_ESPACIO, -DIM_ESPACIO, -DIM_ESPACIO };

	glPushMatrix();
	glRotatef(180, 0, 0, 1);
	quadtex(abajo_derecha_arriba, abajo_izquierda_arriba, abajo_izquierda_abajo, abajo_derecha_abajo, 0.25, 0.5, 0, 0.33, 0, 0);
	glPopMatrix();


	glPopAttrib();
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void init()
// Funcion propia de inicializacion
{
// Mensajes por consola
cout << PROYECTO << " running" << endl;
cout << "Version: OpenGL " << glGetString(GL_VERSION) << endl;
cout << "W: Moverse en dirección frontal de la nave" << endl;
cout << "S: Frenar" << endl;
cout << "A: Rotacion nave hacia la izquierda" << endl;
cout << "D: Rotacion nave hacia la derecha" << endl;
cout << "L: Apagar/Encender luces" << endl;
cout << "B: Mover/Bloquear cabeza " << endl;
cout << "C: Ocultar/Mostrar nave " << endl;
cout << "Click izquierdo: Disparar" << endl;
cout << "Click derecho: Mostrar menu" << endl;
cout << "Rueda hacia arriba: Acelerar paso del tiempo(sin limite)" << endl;
cout << "Rueda hacia abajo: Frenar paso del tiempo(hasta pararse)" << endl;
cout << "Mover ratón: Controlar dirección cámara" << endl;


cout << "esc: Salir" << endl;

engine = createIrrKlangDevice();
engine->play2D("Space.ogg", true);

engineExplosion = engine->addSoundSourceFromFile("explosion.wav");
engineBlaster = engine->addSoundSourceFromFile("Blaster.ogg");


//engine->stopAllSounds();

glClearColor(0,0,0,1.0); // Color de fondo a negro


//glGetFloatv(GL_MODELVIEW_MATRIX, coef); // Inicializa coef a la matriz identidad
//loadTexture();

//CIELO
glGenTextures(1, &texturas[0]);
glBindTexture(GL_TEXTURE_2D, texturas[0]);
cargarImagen((char*)"cielo.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


//LUNA
glGenTextures(1, &texturas[1]);
glBindTexture(GL_TEXTURE_2D, texturas[1]);
cargarImagen((char*)"luna.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//SOL
glGenTextures(1, &texturas[2]);
glBindTexture(GL_TEXTURE_2D, texturas[2]);
cargarImagen((char*)"sol.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//TIERRA
glGenTextures(1, &texturas[3]);
glBindTexture(GL_TEXTURE_2D, texturas[3]);
cargarImagen((char*)"tierra.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


//JUPITER
glGenTextures(1, &texturas[4]);
glBindTexture(GL_TEXTURE_2D, texturas[4]);
cargarImagen((char*)"jupiter.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//MARTE
glGenTextures(1, &texturas[7]);
glBindTexture(GL_TEXTURE_2D, texturas[7]);
cargarImagen((char*)"marte.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//VENUS
glGenTextures(1, &texturas[8]);
glBindTexture(GL_TEXTURE_2D, texturas[8]);
cargarImagen((char*)"venus.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//nave
glGenTextures(1, &texturas[5]);
glBindTexture(GL_TEXTURE_2D, texturas[5]);
cargarImagen((char*)"nave_frontal.png");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

glGenTextures(1, &texturas[9]);
glBindTexture(GL_TEXTURE_2D, texturas[9]);
cargarImagen((char*)"nave_izquierda.png");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

glGenTextures(1, &texturas[10]);
glBindTexture(GL_TEXTURE_2D, texturas[10]);
cargarImagen((char*)"nave_derecha.png");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

glGenTextures(1, &texturas[12]);
glBindTexture(GL_TEXTURE_2D, texturas[12]);
cargarImagen((char*)"elite_dangerous.png");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

glGenTextures(1, &texturas[11]);
glBindTexture(GL_TEXTURE_2D, texturas[11]);
cargarImagen((char*)"nave_abajo.png");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

glGenTextures(1, &texturas[13]);
glBindTexture(GL_TEXTURE_2D, texturas[13]);
cargarImagen((char*)"bala.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//PISTA ATERRIZAJE
glGenTextures(1, &texturas[6]);
glBindTexture(GL_TEXTURE_2D, texturas[6]);
cargarImagen((char*)"pista.jpg");
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

// LUZ NAVE
glLightfv(GL_LIGHT1, GL_AMBIENT, BLANCO);
glLightfv(GL_LIGHT1, GL_DIFFUSE, BLANCO);
glLightfv(GL_LIGHT1, GL_SPECULAR, BLANCO);
glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 2);
glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2);

glLightfv(GL_LIGHT2, GL_AMBIENT, BLANCO);
glLightfv(GL_LIGHT2, GL_DIFFUSE, BLANCO);
glLightfv(GL_LIGHT2, GL_SPECULAR, BLANCO);
glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 2);
glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 2);

glEnable(GL_DEPTH_TEST);
glEnable(GL_LIGHTING);

glEnable(GL_LIGHT0);
glEnable(GL_LIGHT1);
glEnable(GL_LIGHT2);


glShadeModel(GL_SMOOTH);
glEnable(GL_TEXTURE_2D);

glEnable(GL_BLEND);
glEnable(GL_NORMALIZE);

glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);

}
/* Esto al final nada
void dibujar_parte_nave(int textura) {
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBindTexture(GL_TEXTURE_2D, texturas[textura]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glPolygonMode(GL_FRONT, GL_FILL);


	//glRotatef(giro_nave, 0, 1, 0);


	glColor4f(1, 1, 1, 0); //no funciona transparencia
	glBegin(GL_QUADS);
	glTexCoord2f(1, 1);
	glVertex3f(1, 1.5, 1);
	glTexCoord2f(0, 1);
	glVertex3f(-1, 1.5, 1);
	glTexCoord2f(0, 0);
	glVertex3f(-1, 1.5, -1);
	glTexCoord2f(1, 0);
	glVertex3f(1, 1.5, -1);
	glEnd();


	glPopAttrib();
	glPopMatrix();

}*/

void display()
// Funcion de atencion al dibujo
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	
	GLfloat static const posicionLuzArriba[] = { 1, 0, 0, 1 };
	GLfloat direccionLuzArriba[] = { 0.0, 0.0, -1 };
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direccionLuzArriba);
	glLightfv(GL_LIGHT1, GL_POSITION, posicionLuzArriba);

	GLfloat static const posicionLuzAbajo[] = { -1, 0, 0, 1 };
	GLfloat direccionLuzAbajo[] = { 0.0, 0.0, -1 };
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direccionLuzAbajo);
	glLightfv(GL_LIGHT2, GL_POSITION, posicionLuzAbajo);
	

	// Situacion y orientacion de la camara
	Vec3 u = sistema_espacio.getu();
	Vec3 v = sistema_espacio.getv();
	Vec3 w = sistema_espacio.getw();
	Vec3 origen = sistema_espacio.geto(); //origen

	Vec3 punto_mirar = origen - w;
	//gluLookAt(0, 2, 1, 0, 2, 2, 0, 1, 0);
	gluLookAt(origen.x, origen.y, origen.z, punto_mirar.x, punto_mirar.y, punto_mirar.z, v.x, v.y, v.z);

	// Luz ambiental 
	GLfloat luz_ambiente[] = { 1, 1, 1, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luz_ambiente);

	// Luz direccional de sol
	GLfloat posicion_sol[] = { 0.1, 0.1, 0, 1 }; //un poco amarilla
	glLightfv(GL_LIGHT0, GL_POSITION, posicion_sol);


	//quad para los planetas
	GLUquadricObj* quad = gluNewQuadric();
	gluQuadricTexture(quad, GL_TRUE);
	gluQuadricNormals(quad, GLU_SMOOTH);

	//TIERRA
	if (posicionTIERRA[0] < 1000) { // si posicionTIERRA[0] > 1000 significa que esta muerto fuera del cubemap y no lo dibujamos


		glPushMatrix();
		glTranslatef(posicionTIERRA[0], posicionTIERRA[1], 0);

		glRotatef(180, 0, 0, 1);

		//LUNA
		if (posicionLUNA[0] < 1000) {
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glPushMatrix();

			glBindTexture(GL_TEXTURE_2D, texturas[1]);

			GLfloat colorLUNA[] = { 0, 0, 0, 1 };
			GLfloat especularLUNA[] = { 0.2, 0.2,0.2, 1 };
			GLfloat difusoLUNA[] = { 0.3, 0.3, 0.3, 1 };
			GLfloat emisionLUNA[] = { 0.1, 0.1, 0.1, 1 };
			GLfloat brilloLUNA = 2;

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorLUNA);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularLUNA);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusoLUNA);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisionLUNA);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilloLUNA);
			glPolygonMode(GL_FRONT, GL_FILL);
			//glColor3f(1, 1, 1);
			glTranslatef(posicionLUNA[1], posicionLUNA[0], 0);

			gluSphere(quad, 2, 20, 20);

			//glutSolidSphere(10,10,10);
			glPopMatrix();
			glPopAttrib();

		}
		
		//FIN LUNA
		glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texturas[3]);

		GLfloat color[] = { 0, 0, 0, 1 };
		GLfloat especular[] = { 0.2, 0.2, 0.2, 1 };
		GLfloat difuso[] = { 0.5, 0.5, 0.5, 1 };
		GLfloat emision[] = { 0.1, 0.1, 0.1, 1 };
		GLfloat brillo = 2;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difuso);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emision);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brillo);
		glPolygonMode(GL_FRONT, GL_FILL);
		//glColor3f(1, 1, 1);
		glRotatef(rotacionTierra, 0, 0, 1);
		gluSphere(quad, 5, 20, 20);
		glPopMatrix();
		glPopAttrib();
		glPopMatrix();
	}
	
	//FIN TIERRA
	// 
	//JUPITER
	if(posicionJUPITER[0]<1000){  // si posicionJUPITER[0] > 1000 significa que esta muerto fuera del cubemap y no lo dibujamos
	
		glPushMatrix();
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glPushMatrix();

		glTranslatef(posicionJUPITER[0], posicionJUPITER[1], posicionJUPITER[2]);
		glRotatef(180, 0, 0, 1);

		glBindTexture(GL_TEXTURE_2D, texturas[4]);

		GLfloat colorJUPITER[] = { 0, 0, 0, 1 };
		GLfloat especularJUPITER[] = { 0.2, 0.2, 0.2, 1 };
		GLfloat difusoJUPITER[] = { 0.5, 0.5, 0.5, 1 };
		GLfloat emisionJUPITER[] = { 0.1, 0.1, 0.1, 1 };
		GLfloat brilloJUPITER = 1;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorJUPITER);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularJUPITER);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusoJUPITER);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisionJUPITER);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilloJUPITER);
		glPolygonMode(GL_FRONT, GL_FILL);
		//glColor3f(1, 1, 1);
		glRotatef(rotacionJUPITER, 0, 0, 1);
		gluSphere(quad, 20, 20, 20);
		glPopMatrix();



		glPopAttrib();
		glPopMatrix();
	}
	//FIN JUPITER

	//MARTE
	if (posicionMARTE[0] < 1000) { // si posicionMARTE[0] > 1000 significa que esta muerto fuera del cubemap y no lo dibujamos

		glPushMatrix();
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glPushMatrix();

		glTranslatef(posicionMARTE[0], posicionMARTE[1], posicionMARTE[2]);
		glRotatef(180, 0, 0, 1);

		glBindTexture(GL_TEXTURE_2D, texturas[7]);

		GLfloat colorMARTE[] = { 0, 0, 0, 1 };
		GLfloat especularMARTE[] = { 0.4, 0.2, 0.2, 1 };
		GLfloat difusoMARTE[] = { 0.8, 0.5, 0.5, 1 };
		GLfloat emisionMARTE[] = { 0.3, 0.1, 0.1, 1 };
		GLfloat brilloMARTE = 3;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorMARTE);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularMARTE);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusoMARTE);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisionMARTE);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilloMARTE);
		glPolygonMode(GL_FRONT, GL_FILL);
		//glColor3f(1, 1, 1);
		glRotatef(rotacionMARTE, 0, 0, 1);
		gluSphere(quad, 3, 20, 20);
		glPopMatrix();



		glPopAttrib();
		glPopMatrix();
	}
	//FIN MARTE

	//VENUS
	if (posicionVENUS[0] < 1000) {// si VENUS[0] > 1000 significa que esta muerto fuera del cubemap y no lo dibujamos


		glPushMatrix();
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glPushMatrix();

		glTranslatef(posicionVENUS[0], posicionVENUS[1], posicionVENUS[2]);
		glRotatef(180, 0, 0, 1);

		glBindTexture(GL_TEXTURE_2D, texturas[8]);

		GLfloat colorVENUS[] = { 0, 0, 0, 1 };
		GLfloat especularVENUS[] = { 0.2, 0.2, 0.2, 1 };
		GLfloat difusoVENUS[] = { 0.5, 0.5, 0.5, 1 };
		GLfloat emisionVENUS[] = { 0.1, 0.1, 0.1, 1 };
		GLfloat brilloVENUS = 10;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorVENUS);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularVENUS);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusoVENUS);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisionVENUS);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilloVENUS);
		glPolygonMode(GL_FRONT, GL_FILL);
		//glColor3f(1, 1, 1);
		glRotatef(rotacionVENUS, 0, 0, 1);
		gluSphere(quad, 4, 20, 20);
		glPopMatrix();



		glPopAttrib();
		glPopMatrix();
	}
	//FIN VENUS

	//SOL
	if (posicionSOL[0] < 1000){

	
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glPushMatrix();

		glTranslatef(posicionSOL[0], posicionSOL[1], posicionSOL[2]);
		glBindTexture(GL_TEXTURE_2D, texturas[2]);

		GLfloat colorSOL[] = { 1, 1, 0, 1.0f };
		GLfloat especularSOL[] = { 0, 0, 0, 1 };
		GLfloat difusoSOL[] = { 0.5, 0.5, 0.5, 1.0f };
		GLfloat emisionSOL[] = { 0.1, 0.1, 0.1, 1.0f };
		GLfloat brilloSOL = 100;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorSOL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularSOL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusoSOL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisionSOL);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilloSOL);
		glPolygonMode(GL_FRONT, GL_FILL);

		//glColor3f(1, 1, 0);
		gluSphere(quad, 20, 20, 20);

		//glutSolidSphere(50,50,50);
		glPopMatrix();
		glPopAttrib();
	}
	//FIN SOL
	generar_cubemap(0);

	

	//Pista 
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glBindTexture(GL_TEXTURE_2D, texturas[6]);


	GLfloat colorPISTA[] = { 0.5, 0.5, 0.5, 1 };
	GLfloat especularPISTA[] = { 0.2, 0.2,0.2, 1 };
	GLfloat difusoPISTA[] = { 0.3, 0.3, 0.3, 1 };
	GLfloat emisionPISTA[] = { 0.2, 0.2, 0.2, 1 };
	GLfloat brilloPISTA = 1;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorPISTA);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularPISTA);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusoPISTA);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisionPISTA);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilloPISTA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glPolygonMode(GL_FRONT, GL_FILL);

	//glColor4f(1, 1, 1, 1); //no funciona transparencia
	glTranslatef(100, -300, 17.9);
	glScalef(20, 20, 20);
	glRotatef(-90, 1, 0, 0);
	GLfloat v0[] = { 1,1,2 };
	GLfloat v1[] = { -1,1, 2 };
	GLfloat v2[] = { -1,1, -2  };
	GLfloat v3[] = { 1,1, -2  };
	quadtex(v0, v1, v2, v3, 0, 15, 0, 15, 500, 500);
	
	glPopAttrib();
	glPopMatrix();

	
	for (int i = 0; i < proyectiles.size(); i++) {
		glPushMatrix();
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glBindTexture(GL_TEXTURE_2D, texturas[13]);


		GLfloat colorBALA[] = { 1, 0, 0, 1 };
		GLfloat especularBALA[] = { 1, 0.2,0.2, 1 };
		GLfloat difusoBALA[] = { 1, 1, 1, 1 };
		GLfloat emisionBALA[] = { 1, 0.2, 0.2, 1 };
		GLfloat brilloBALA = 300;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorBALA);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularBALA);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusoBALA);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisionBALA);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilloBALA);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glPolygonMode(GL_FRONT, GL_FILL);
		proyectil bala = proyectiles[i];
		glTranslatef(bala.posicion.x, bala.posicion.y, bala.posicion.z);
		glColor3f(1, 0, 0);
		glutSolidSphere(0.5, 20, 20);
		glPopAttrib();
		glPopMatrix();
	}
	
	//nave lo ultimo para la tranparencia

	if (mostrarNave) {
		/*esta nave es en 3d mas o menos
		glPushMatrix();
		//FRONTAL
		glPushMatrix();
		glTranslatef(origen.x - punto_mirar.x, origen.y - punto_mirar.y + 1, origen.z - punto_mirar.z);
		glTranslatef(origen.x, origen.y, origen.z);
		dibujar_parte_nave(5);
		glPopMatrix();

		//IZQUIERDA
		glPushMatrix();
		glTranslatef(origen.x - punto_mirar.x - 0.25, origen.y - punto_mirar.y + 1, origen.z - punto_mirar.z);
		glTranslatef(origen.x, origen.y, origen.z);
		glRotatef(60, 0, 0, 1);
		dibujar_parte_nave(9);
		glPopMatrix();

		//DERECHA
		glPushMatrix();
		glTranslatef(origen.x - punto_mirar.x + 0.25, origen.y - punto_mirar.y + 1, origen.z - punto_mirar.z);
		glTranslatef(origen.x, origen.y, origen.z);
		glRotatef(-60, 0, 0, 1);
		dibujar_parte_nave(10);
		glPopMatrix();
		//ABAJO
		glPushMatrix();
		glTranslatef(origen.x - punto_mirar.x , origen.y - punto_mirar.y+1.30 , origen.z - punto_mirar.z);
		glTranslatef(origen.x, origen.y, origen.z);
		glRotatef(-45, 1, 0, 0);
		dibujar_parte_nave(11);
		glPopMatrix();

		glPopMatrix();
		*/
	glPushMatrix();
	//(90, 0, 0, 1);
		
	glTranslatef(origen.x,origen.y+1,origen.z);
	glScalef(0.001, 0.001, 0.001);
	generar_cubemap(12);
	glPopMatrix();
	
	}
	
	glutSwapBuffers();
}
void reshape(GLint w, GLint h)
// Funcion de atencion al redimensionamiento
{
	window_width = glutGet(GLUT_WINDOW_WIDTH)/2;
	window_height = glutGet(GLUT_WINDOW_HEIGHT)/2;
	// Usamos toda el area de dibujo
	glViewport(0, 0, w, h);
	// Definimos la camara (matriz de proyeccion)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float razon = (float)w / h;
	/* CAMARA PERSPECTIVA */
	gluPerspective(45, razon, 0.1, 10000);
}

void reset() {
	sistema_espacio = Sistema3d(
		Vec3(1, 0, 0),	// u / x
		Vec3(0, 0, 1),	// v / y
		Vec3(0, -1, 0),	// w / z
		Vec3(100, -300, 0)); // origen o posicion inicial
	velocidad_actual = 0;
	

}
void mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		velocidad_tiempo += 0.01;
	}else{
		velocidad_tiempo -= 0.01;
		velocidad_tiempo = max(0, velocidad_tiempo);
	}

	return;
}
void onClick(int button, int state, int x, int y)
// Funcion de atencion al boton del raton
// button: GLUT_LEFT|MIDDLE|RIGHT_BUTTON
// state: GLUT_UP|DOWN
// x,y: pixel respecto a vertice superior izquierd
 {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		engine->play2D(engineBlaster);
		Vec3 pos_inicial = sistema_espacio.geto().normalize();
		Vec3 direccion = sistema_espacio.getw().normalize().operator*(-1);
		pos_inicial = pos_inicial + direccion;
		pos_inicial = pos_inicial + sistema_espacio.geto();
		//cout << direccion.x<<','<<direccion.y<<','<<direccion.z << endl;
		//pos_inicial.y += 1;
		proyectil nuevo_proyectil;
		nuevo_proyectil.posicion = pos_inicial;
		nuevo_proyectil.direccion = direccion;
		proyectiles.push_back(nuevo_proyectil);
	}
}
void onMotion(int x, int y)
// Funcion de atencion al raton con el boton pulsado
// x,y: coordenadas del cursor referidas al pixel superior izquierdo(0,0)
{
		
	// cabeceo
	cabeceoVertical = (window_height - y) * (1 / float(glutGet(GLUT_WINDOW_HEIGHT)));
	sistema_espacio.rotar(cabeceoVertical, sistema_espacio.getu());

	// guiñada
	giroHorizontalCamara = (window_width - x)  * (1 / float(glutGet(GLUT_WINDOW_WIDTH)));
	sistema_espacio.rotar(giroHorizontalCamara, sistema_espacio.getv());
	
	glutWarpPointer(window_width, window_height);
	
	//cout << relativeWidth << ","<< relativeHeight << "," << x << "," << y << endl;
	//cout << giroHorizontalCamara << endl;
	
}
void onKey(unsigned char tecla, int x, int y)
// Funcion de atencion al teclado
{
	
	float cantidadGiro = 0.03;
	switch (tecla) {
	case 'w': // Vuelve a la posicion original
		velocidad_actual += aceleracion_actual * tiempo_transcurrido;
		velocidad_actual = min(velocidad_actual, max_velocidad);
		break;
	case 's':
		velocidad_actual -= aceleracion_actual * tiempo_transcurrido;
		velocidad_actual = max(velocidad_actual, -2);
		break;
	case 'd':
		giroz = -cantidadGiro;
		giro_nave += 1.63;
		sistema_espacio.rotar(giroz, sistema_espacio.getw());
		break;
	case 'a':
		giroz = cantidadGiro;
		giro_nave -= 1.63;
		sistema_espacio.rotar(giroz, sistema_espacio.getw());
		break;
	case 'c':
		mostrarNave = not mostrarNave;
		break;
	case 'l':
		lucesEncendidas = not lucesEncendidas;
		if (lucesEncendidas) {
			glEnable(GL_LIGHT1);
			glEnable(GL_LIGHT2);
		}
		else {
			glDisable(GL_LIGHT1);
			glDisable(GL_LIGHT2);
		}
		break;
	case 'b':
		//engine->play2D(engineExplosion);
		bloquear_cabeza = !bloquear_cabeza;
		sistema_espacio = sistema_nave;

		break;
	case 27: // Salir de la aplicacion
		exit(0);
	}
	//cout << aceleracion_actual << "," << velocidad_actual << endl;

	
	glutPostRedisplay();
	
}

void onTimer(int valor)
// Funcion de atencion al timer periodico
{
	glutTimerFunc(valor, onTimer, valor); // Se encola un nuevo timer

	static int antes = glutGet(GLUT_ELAPSED_TIME);
	int ahora = glutGet(GLUT_ELAPSED_TIME);

	tiempo_transcurrido = (ahora - antes) / 1000.0f;

	Vec3 nueva_posicion = sistema_nave.getw().operator*(velocidad_actual);

	sistema_espacio.seto(sistema_espacio.geto().operator-(nueva_posicion));

	if (bloquear_cabeza) {
		sistema_nave = sistema_espacio;
	}
	else {
		sistema_nave.seto(sistema_espacio.geto());

	}

	for (int i = 0; i < proyectiles.size(); i++) {
		proyectiles[i].posicion = proyectiles[i].posicion + proyectiles[i].direccion.operator*(velocidad_proyectil);

	}


	//cout << tiempo_transcurrido << endl;
	antes = ahora;
	glutPostRedisplay(); // Se manda el dibujo
}
bool comprobar_colision(float x1, float y1, float z1, float x2, float y2, float z2, float distancia) {
	if (sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2)) <= distancia) {
		return true;
	}
	return false;
}
void onIdle()
// Funcion de atencion al evento idle
{
	//Calculamos el tiempo transcurrido desde la última vez
	static int antesidle = 0;
	int ahora, tiempo_transcurrido;
	ahora = glutGet(GLUT_ELAPSED_TIME); //Tiempo transcurrido desde el inicio
	tiempo_transcurrido = ahora - antesidle; //Tiempo transcurrido desde antes en msg.
	tiempo_transcurrido *= velocidad_tiempo;
	// angulo = angulo anterior + velocidad x tiempo
	anguloLUNA += velocidad * tiempo_transcurrido / 1000.0;
	posicionLUNA[0] = radioLUNA * sin(anguloLUNA);
	posicionLUNA[1] = radioLUNA * cos(anguloLUNA);

	if (posicionTIERRA[0]<1000) {
		anguloTIERRA += velocidadTIERRA * tiempo_transcurrido / 1000.0;
		posicionTIERRA[0] = radioTIERRA * sin(anguloTIERRA);
		posicionTIERRA[1] = radioTIERRA * cos(anguloTIERRA);
		rotacionTierra += velocidadRotacionTIERRA * tiempo_transcurrido / 1000;
	}
	else {
		posicionTIERRA[0] = 2000;
	}
	
	if (posicionJUPITER[0] < 1000) {
	anguloJUPITER += velocidadJUPITER * tiempo_transcurrido / 1000.0;
	posicionJUPITER[0] = radioJUPITER * sin(anguloJUPITER);
	posicionJUPITER[1] = radioJUPITER * cos(anguloJUPITER);
	rotacionJUPITER += velocidadRotacionJUPITER * tiempo_transcurrido / 1000;
	}
	else {
		posicionJUPITER[0] = 2000;
	}
	anguloMARTE += velocidadMARTE * tiempo_transcurrido / 1000.0;
	posicionMARTE[0] = radioMARTE * sin(anguloMARTE);
	posicionMARTE[1] = radioMARTE * cos(anguloMARTE);
	rotacionMARTE += velocidadRotacionMARTE * tiempo_transcurrido / 1000;

	anguloVENUS += velocidadVENUS * tiempo_transcurrido / 1000.0;
	posicionVENUS[0] = radioVENUS * sin(anguloVENUS);
	posicionVENUS[1] = radioVENUS * cos(anguloVENUS);
	rotacionVENUS += velocidadRotacionVENUS * tiempo_transcurrido / 1000;
	

	Vec3 origen = sistema_espacio.geto();
	//COLISION JUPITER
	if (posicionJUPITER[0]<1000 && comprobar_colision(origen.x,origen.y,origen.z, posicionJUPITER[0], posicionJUPITER[1], posicionJUPITER[2],20)) {
		//engine->getPlayLength();
		engine->play2D(engineExplosion);
		reset();
	}
	//COLISION TIERRA
	if (posicionTIERRA[0] < 1000 && comprobar_colision(origen.x, origen.y, origen.z, posicionTIERRA[0], posicionTIERRA[1], posicionTIERRA[2], 5)) {
		engine->play2D(engineExplosion);
		reset();
	}
	//COLISION LUNA
	if (posicionLUNA[0] < 1000 && comprobar_colision(origen.x, origen.y, origen.z, posicionLUNA[0], posicionLUNA[1], posicionLUNA[2], 2)) {
		engine->play2D(engineExplosion);
		reset();
	}
	//COLISION VENUS
	if (posicionVENUS[0] < 1000 && comprobar_colision(origen.x, origen.y, origen.z, posicionVENUS[0], posicionVENUS[1], posicionVENUS[2], 4)) {
		engine->play2D(engineExplosion);
		reset();
	}
	//COLISION MARTE
	if (posicionMARTE[0] < 1000 && comprobar_colision(origen.x, origen.y, origen.z, posicionMARTE[0], posicionMARTE[1], posicionMARTE[2], 3)) {
		engine->play2D(engineExplosion);
		reset();
	}
	//COLISION SOL
	if (posicionSOL[0] < 1000 && comprobar_colision(origen.x, origen.y, origen.z, 0, 0, 0, 20)) {
		engine->play2D(engineExplosion);
		reset();
	}

	for (int i = 0; i < proyectiles.size(); i++) {
		Vec3 origen_proyectil = proyectiles[i].posicion;
		if (posicionJUPITER[0] < 1000 && comprobar_colision(origen_proyectil.x, origen_proyectil.y, origen_proyectil.z, posicionJUPITER[0], posicionJUPITER[1], posicionJUPITER[2], 20)) {
			//engine->getPlayLength();
			engine->play2D(engineExplosion);
			posicionJUPITER[0] = 2000;
			posicionJUPITER[1] = 2000;
			posicionJUPITER[2] = 2000;
		}
		//COLISION TIERRA
		if (posicionTIERRA[0] < 1000 && comprobar_colision(origen_proyectil.x, origen_proyectil.y, origen_proyectil.z, posicionTIERRA[0], posicionTIERRA[1], posicionTIERRA[2], 5)) {
			engine->play2D(engineExplosion);
			posicionTIERRA[0] = 2000;
			posicionTIERRA[1] = 2000;
			posicionTIERRA[2] = 2000;
			posicionLUNA[0] = 2000;
			posicionLUNA[1] = 2000;
			posicionLUNA[2] = 2000;
		}
		//COLISION LUNA
		if (posicionLUNA[0] < 1000 && comprobar_colision(origen_proyectil.x, origen_proyectil.y, origen_proyectil.z, posicionLUNA[0], posicionLUNA[1], posicionLUNA[2], 2)) {
			engine->play2D(engineExplosion);
			posicionLUNA[0] = 2000;
			posicionLUNA[1] = 2000;
			posicionLUNA[2] = 2000;
		}
		//COLISION VENUS
		if (posicionVENUS[0] < 1000 && comprobar_colision(origen_proyectil.x, origen_proyectil.y, origen_proyectil.z, posicionVENUS[0], posicionVENUS[1], posicionVENUS[2], 4)) {
			engine->play2D(engineExplosion);
			posicionVENUS[0] = 2000;
			posicionVENUS[1] = 2000;
			posicionVENUS[2] = 2000;
		}
		//COLISION MARTE
		if (posicionMARTE[0] < 1000 && comprobar_colision(origen_proyectil.x, origen_proyectil.y, origen_proyectil.z, posicionMARTE[0], posicionMARTE[1], posicionMARTE[2], 3)) {
			engine->play2D(engineExplosion);
			posicionMARTE[0] = 2000;
			posicionMARTE[1] = 2000;
			posicionMARTE[2] = 2000;
		}
		//COLISION SOL
		if (comprobar_colision(origen_proyectil.x, origen_proyectil.y, origen_proyectil.z, 0, 0, 0, 20)) {
			engine->play2D(engineExplosion);
			posicionSOL[0] = 2000;
			posicionSOL[1] = 2000;
			posicionSOL[2] = 2000;
			glDisable(GL_LIGHT0);

		}
	}

	

	antesidle = ahora;
	glutPostRedisplay();
}



void onMenu(int opcion)
// Funcion de atencion al menu de popup
{
	switch (opcion) {
	case 0:
		
		break;
	case 1:
		engine->stopAllSounds();
		break;

	case 2:
		mostrarNave = not mostrarNave;
		break;
	case 3:
		lucesEncendidas = not lucesEncendidas;
		if (lucesEncendidas) {
			glEnable(GL_LIGHT1);
			glEnable(GL_LIGHT2);
		}
		else {
			glDisable(GL_LIGHT1);
			glDisable(GL_LIGHT2);
		}
		break;
	case 4:
		engine->stopAllSounds();
		engine->play2D("Space.ogg", true);
		break;
	case 5:
		engine->stopAllSounds();
		engine->play2D("clasica.ogg", true);
		break;
	case 6:
		engine->stopAllSounds();
		engine->play2D("getout.ogg", true);
	}
	
	glutPostRedisplay();
}

void initMenu()
// Construye el menu de popup
{
	int menuMusica = glutCreateMenu(onMenu);
	glutAddMenuEntry("Outer wilds", 4);
	glutAddMenuEntry("Clásica", 5);
	glutAddMenuEntry("Getout", 6);
	glutCreateMenu(onMenu);
	glutAddSubMenu("PONER MUSICA", menuMusica);
	glutAddMenuEntry("QUITAR MUSICA", 1);
	glutAddMenuEntry("OCULAR/MOSTRAR nave", 2);
	glutAddMenuEntry("APAGAGAR/ENCENDER LUCES", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON); // Ya no pasa por onClick
}
void main(int argc, char** argv)
// Programa principal
{
	std::srand(std::time(0));

	glutInit(&argc, argv); // Inicializacion de GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Alta de buffers a usar
	glutInitWindowSize(window_width, window_height); // Tamanyo inicial de la ventana
	glutCreateWindow(PROYECTO); // Creacion de la ventana con su titulo
	glutDisplayFunc(display); // Alta de la funcion de atencion a display
	glutReshapeFunc(reshape); // Alta de la funcion de atencion a reshape
	glutMouseFunc(onClick); // Alta de la funcion de atencion al click del ratón
	glutPassiveMotionFunc(onMotion); // Alta de la funcion de atencion al movimiento del ratón
	glutTimerFunc(1000 / tasaFPS, onTimer, tasaFPS); // Alta de la funcion de atencion al timer
	glutKeyboardFunc(onKey); // Alta de la funcion de atencion al teclado
	glutIdleFunc(onIdle);
	glutMouseWheelFunc(mouseWheel);

	initMenu(); // Construye menus

	glutSetCursor(GLUT_CURSOR_FULL_CROSSHAIR);
	init(); // Inicializacion propia
	glutMainLoop(); // Puesta en marcha del programa
	FreeImage_DeInitialise();
}