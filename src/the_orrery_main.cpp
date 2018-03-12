
#define _WIN32_WINNT 0x0500

#include <stdio.h>	        // biblioteka IO
#include <sstream>			// klasa sstream pomocna przy konwersji liczby na tekst
#include <math.h>           // biblioteka mateamtycza
#include <time.h>           // biblioteka funkcji czasowych 
#include <windows.h>		// Plik nag≥Ûwkowy Windows	
#include <windowsx.h>		// Plik nag≥Ûwkowy Windows
#include <gl\gl.h>			// Pliki nag≥Ûwkowe OpenGL
#include <gl\Gl.h>
#include <gl\glu.h>			
#include <gl\glaux.h>
#include <dsound.h>         // Pliki nag≥Ûwkowe DirectSound
#include <dmksctrl.h>      
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
#include "TextureLoader.h"  // Klasa do ≥adowania textur z plikÛw (autor: Chris Leathley)
#include "t3dlib3.h"        // Klasa opakowujπca funkcje DirectSound (autor: Andre LaMothe)

// STA£E MATEMATYCZNE WYKORZYSTYWANE W NIEKT”RYCH OBLICZENIACH

#define PI      3.141592654
#define PID2    1.570796327
#define TWOPI   6.283185308

// STA£E OKREåLAJ•CE ROZMIAR OKNA I G£ BI  KOLOR”W

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768
#define COLOR_DEPTH     32

// STA£E OKREåLAJ•CE SZCZEG”£OWOWå∆ TR”JWYMIAROWYCH OBIEKT”W

#define PLANET_PRECISION  40
#define ORBIT_PRECISION   300

// PROMIE— KULI TWORZ•CEJ T£O

#define STARS_RADIUS      200

HWND main_window_handle;    // Kontekst urzπdzenia okna wykorzystywany przez blibliotekÍ t3dlib

HDC			hDC=NULL;		// Kontekst urzπdzenia okna
HGLRC		hRC=NULL;		// G≥Ûwny kontekst renderowania (OpenGL)
HWND		hWnd=NULL;		// Uchwyt do okna programu
//HWND		hWnd2=NULL;		// Uchwyt do okna programu
HINSTANCE	hInstance;		// Uchwyt do instancji programu

bool active=TRUE;		     // Flaga zaznaczajπca czy okno jest aktywnie (zminimalizowane, czy nie)
bool keys[256];			     // Tablica do odczytu stanu klawiatury
bool lmb_pressed;            // Czy jest wciúniÍty lewy klawisz miszy
bool rmb_pressed;            // Czy jest wciúniÍty prawy kalwisz myszy
bool lmb_released;           // Czy zosta≥ zwolniony lewy klawisz myszy
int keyboard_delay=0;        // Zmienna s≥uøπca do sterowania opuünienia z jakim sπ odczytywane znaki z klawiatury
int year_digit_editing;      // KtÛra cyfra roku jest w≥aúnie edytowana, gdy edytujemy rok przy pomocy klawiatury
int delta;                   // IloúÊ pixeli o jakπ jest przesuniÍty kursor wzglÍdem lewej krawÍdzi suwaka
int  mouse_x;                // WspÛ≥rzÍdna x kursora
int  mouse_y;                // WspÛ≥rzÍdna y kursora
int old_mouse_x;             // Poprzednia wspÛ≥rzÍdna x kursora
int old_mouse_y;             // Poprzednia wspÛ≥rzÍdna y kursora
int  mouse_delta;            // Odchylenie rolki myszy
int  old_mouse_delta;        // Poprzednie odchylenie rolki myszy
int  mouse_roll;             // Czy rolka zosta≥a odchylona w przÛd (1) czy w ty≥ (-1)
bool no_click;               // Czy nie zotsa≥ wciúniÍty øaden element interface'u (mimo klikniÍci myszπ)

// GLuint texture[10];	        // Identyfikatory tekstur

// STRUKTURA PRZECHOWUJ•CA PARAMETRY ORBITY
typedef  struct {
	long double a;           // årednia odleg≥oúÊ od S≥oÒca (jednostka: AU)
	long double a_pc;        // (jednostka: AU/wiek)
	long double e;           // MimoúrÛd (jednostka: radiany)
	long double e_pc;        //(jednostka: radianÛw/wiek)
	long double I;           // Nachylenie orbity wzglÍdem ekliptyki (jednostka: stopnie)
	long double I_pc;	     //(jednostka: stopni/wiek)
	long double L;           //årednia d≥ugoúÊ ekliptyczna (jednostka: stopnie)
	long double L_pc;        //(jednostka: stopni/wiek)
	long double W;           //D≥ugoúÊ peryhelionu (jednostka: stopnie)
	long double W_pc;        //(jednostka: stopni/wiek)
	long double O;           //D≥ugoúÊ punktu wznoszenia (jednostka: stopnie)
	long double O_pc;        //(jednostka: stopni/wiek)
	long double b;
	long double c;
	long double s;
	long double f;
} sOrbit;                  

// PUNKT 2D

typedef  struct{
	long double x;
	long double y;
} sPoint2D;

// PUNKT 3D

typedef  struct{
	long double x;
	long double y;
	long double z;
} sPoint3D;

// KLASA ZEGARA

class cClock {
public:
	bool bc;			// era (p.n.e = true n.e.=false)
	int year;           // rok
	int month;          // miesiπc
	int day;            // dzieÒ
	int hour;           // godzina
	int minute;         // minuta
	int second;         // sekunda
	int millisecond;    // milisekunda
	long double julian_day();            //Konwersja czasu na dni juliaÒskie.
	void inc_time(long double msec);     //ZwiÍksza zegar o zadanπ liczbÍ milisekund.
	void set_current_time();             //Ustawia zegar na czas systemowy.
}; 

// KLASA £ADUJ•CA BITMAPY DO PAMI CI, UMOØLIWIAJ•CA RYSOWANIE ICH NA EKRANIE LUB WYKORZYTSANIE JAKO TEKSTURY

class cBMP {
private:
	glTexture txBitmap;			// Zmienna przechowujπca teksturÍ
	TextureLoader Bitmap;       // Klasa ≥adujπca tekstury z plikÛw i zarzπdzajπca jej atrybutami
	bool isLoaded;				// Czy zosta≥ wczytany jakiú plik
public:
	cBMP();						// Konstruktor zeruje pola klasy
	int width;					// SzerokoúÊ bitmapy z jakπ ma byÊ ona wyúwietlona na ekranie
	int height;					// WysokoúÊ bitmapy z jakπ ma byÊ ona wyúwietlona na ekranie
	int GetHeight();			// Zwraca wartoúÊ height
	int GetWidth();				// Zwraca wartoúÊ width
	GLuint GetID();				// Zwraca identyfikator tekstury w OpenGL
	void LoadBMP(char* filename, bool resize, int new_width, int new_height);	//£aduje bitmapÍ z pliku
	void PrintBMP(int x_pos, int y_pos);	//Wyúwietla bitmapÍ na ekranie w punkcie o wspÛ≥rzednych x_pos, y_pos
	~cBMP();					// Destruktor usuwa bitmapÍ z pamiÍci jeúli zosta≥a ona wczyta
};

// KLASA OPAKOWUJ•CA KLAS  cBMP W FUNKCJONALNOåCI• PRZYCISKU

class cButton {
private:
	bool bActive;	          // Flaga oznaczajπca ìpozycjÍî przycisku (w≥/wy≥).
	cBMP* ButtonActive;       // Zestaw trzech bitmap przedstawiajπcych 
	cBMP* ButtonNotActive;    // przycisk w kaødym z trzech stanÛw: 
	cBMP* ButtonPressed;      // w≥πczonym, wy≥πczonym i dodatkowym stanem gdy przycisk jest wciskany.
public:
	RECT rcArea;											// Prostokπtny obszar przycisku.
	void InitButton(RECT rectangle, cBMP* active, cBMP* not_actvie, cBMP* pressed);	// Ustawia pola klasy
	void PrintButton(bool pressed, int x_pos, int y_pos);	// Wyúwietla przycisk na ekranie
	bool IsInArea(int,int);									// Czy dany punkt znajduje siÍ nad przyciskiem
	bool IsActive();										// Czy przycisk jest w≥πczony 
	void Enable();											// W≥πcz przycisk
	void Disable();											// Wy≥πcz przycisk
};

// TYP U£ATWIAJ•CY KORZYSTANIE Z TABLICY PRZYCISK”W, KAØDEMU PRZYCISKOWI W PROGRAMIE ODPOWIADA ETYKIETKA

typedef enum {BT_EXIT, BT_INFO, BT_MUSIC, BT_SOUND, BT_ORBIT, BT_LABELS, BT_BIRDVIEW, BT_BCKG, BT_PAUSE, BT_SLIDE, BT_RESET_SPEED, BT_RESET_TIME} enButtons;

// TYP U£ATWIAJ•CY KORZYSTANIE Z TABLICY ELEMENT”W ZEGARA

typedef enum {CH_AGE, CH_YEAR, CH_MONTH, CH_DAY, CH_HOUR, CH_MINUTE, CH_SECONDS} enClockHighlights;

// WIRTUALNA KAMERA OKREåLAJ•CA PUNKT Z KT”REGO JEST WIDOCZNA SCENA

struct cCamera{
		sPoint3D focus;						// Punkt na ktÛry patrzy kamera
		sPoint3D old_focus;					// Stary punkt na ktÛry patrzy kamera
		int focus_interpolation;			// Zmienna sterujπca interpolacjπ punktu na ktÛry patrzy kamera i minimalnego promienia
        long double alpha;					// WspÛ≥rzÍdna biegunowa pozycji kamera
        long double theta;					// WspÛ≥rzÍdna biegunowa pozycji kamera
        long double radius;					// Odleg≥oúÊ z jakiej patrzy kamera na punkt centralny
		long double inf_radius;				// Minimalny promieÒ (s≥uøy po to by nie moøna by≥o zajrzeÊ do wnÍtrza planety
		long double old_inf_radius;			// Poprzedni minimalny promieÒ
		void change_focus(double long x, double long y, double long z, double long r); // Interpoluje punkt ku ktÛremu jest zwrÛcona kamera oraz minimalny promieÒ
		void set_focus(double long x, double long y, double long z, double long r);    // Ustawia punkt centralny na (x,y,z), a minimalny promieÒ na r
};

// KLASA PRZECHOWUJ•CA INFORMACJE DT. PLANETY, UMOØLIWIA OBLICZENIE JEJ POZYCJI I WYåWIETLENIE NA EKRANIA

class cPlanet {
private:
	sOrbit orbit;                                 // Orbita
	sPoint3D orbit_line[ORBIT_PRECISION];         // Lista punktÛw z ktÛrych jest z≥oøona linia reprezentujπca orbite planety
	long double diameter;                         // årednica (j: AU)
	long double rotation_speed;                   // D≥ugoúÊ jednego dnia (j: dni ziemskie)
	long double sun_year;                         // Rok s≥oneczny (j:dni ziemskie)
	long double axial_tilt;                       // Nachylenie osi (j: stopnie)
	sPoint3D sun_position;                        // Pozycja wzgledem s≥oÒca
	sPoint3D screen_position;                     // Pozycja ekranowa. (j: pixele)
public:
	bool set_orbit(double long a,double long a_pc,double long b,double long c,double long e,double long e_pc,double long f,double long I,double long I_pc,double long L,double long L_pc,double long O,double long O_pc,double long s,double long W,double long W_pc);  //Inicjuje orbitÍ podanymi wartoúciami
	bool generate_orbit_line();									// Oblicza punkty tworzπce liniÍ orbitÍ
	sPoint3D get_orbit_line(int num);							// Zwraca m-ty punkt tworzπcy liniÍ orbity
	bool calculate_sun_positions(long double jd);				// Oblicza pozycjÍ wzglÍdem s≥oÒca
	bool calculate_screen_position(sPoint3D move);				// Oblicza pozycjÍ ekranowπ na podstawie pozycji wzglÍdem s≥oÒca przesuniÍtej o wektor move
	bool set_diameter(long double new_diameter);				// Ustawia úrednice
	long double get_diameter();									// Zwraca úrednicÍ
	bool set_rotation_speed(long double new_rotation_speed);	// Ustawia prÍdkoúÊ rotacji
	long double get_rotation_speed();							// Zwraca prÍdkoúÊ rotacji
	bool set_sun_year(long double new_sun_year);				// Ustawia d≥ugoúÊ roku s≥onecznego (wartoúÊ ta jest potrzebna do obliczenia punktÛw tworzπcych orbitÍ)
	long double get_sun_year();									// Zwraca d≥ugoúÊ roku
	bool set_axial_tilt(long double new_axial_tilt);			// Ustawia nachylenie osi
	long double get_axial_tilt();								// Zwraca nachylenie osi
	sPoint3D get_sun_position();								// Zwraca pozycjÍ planety wzglÍdem s≥oÒca
	sPoint3D get_screen_position();								// Zwraca pozycjÍ ekranowπ
};

cPlanet planet[10];				// Tablica 10 planet (0-s≥oÒce)
cBMP saturn_ring;				// Bitmapa przechowujπca obraz pierúcieni saturna
cBMP title;						// Bitmapa z tytu≥em programu
int selected;					// Numer aktualnie zaznaczonej planety
cCamera camera;					// Wirtualna kamera
cBMP planet_textures[10];		// Tekstury planet
cClock timer;					// Zegar programowy
int time_speed=0;				// PrÍdkoúÊ up≥ywu czasu
bool isClockEditing;			// Czy zegar jest edytowany
cBMP stars_bkg;					// Bitmapa t≥a
int sound_labels[10];			// Etykietki düwiÍkowe planet
int sound_slide;				// DüwiÍk sywaka
int sound_button;				// DüwiÍk przycisku
int sound_timer_edit;			// DüwiÍk edytowanego zegara
int sound_timer_edit_error;		// DüwiÍk b≥Ídu przy edycji zegara
int music;						// Muzyka

struct {
	cBMP InfoWindows[11];        // Bitmapy wyúwietlane jako okna z informacjami.
	cBMP Panel;                  // Belka na ktÛrej umieszczony jest suwak i przyciski.
	cBMP ClockDigits[10];        // Bitmapy zawierajπce cyfry uøywane przy wyúwietlaniu daty.
	cBMP ClockMonths[12];        // Bitmapy zawierajπce nazwy miesiÍcy.
	cBMP ClockBC[2];             // Bitmapy zawierajπce nazwy er.
	cBMP ClockHighlightGFX[5];   // Bitmapy zaznaczajπce dany element zegara przy edycji.
	cBMP LabelsGFX[10];			 // Etykietki z nazwami planet
	cButton Labels[10];			 // Interaktywne etykietki z nazwami planet
	cButton ClockHighlight[7];   // Guziki poúredniczπce przy zaznaczaniu elementÛw zegara
	cButton Buttons[12];         // Klasy reprezentujπce wszystkie przyciski interfaceíu
	cBMP ButtonsGFX[12][3];      // Bitmapy przechowujπce obrazki wykorzystywane przez klasy z tablicy Buttons.
} Interface;

// FUNKCJA ZAMIENIA WSP”£RZ DNE SCENY NA WSP”£RZ DNE EKRANOWE

sPoint3D GlToWin(sPoint3D obj){
	GLdouble result[3];
	sPoint3D result2;
	// Macierze przekszta≥ceÒ
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLdouble win_x,win_y,win_z;
	// Pobranie macierzy przekszta≥ceÒ
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);
	// Obliczenie wyniku
	gluProject(obj.x,obj.y,obj.z,modelview,projection,viewport,&result[0],&result[1],&result[2]);
	// Modyfikacja wyniku o wysokoúÊ okna zwiπzana z tym iø oú Y w Windows biegnie od gÛrnego lewego rogu w dÛ≥
	result[1]=WINDOW_HEIGHT-result[1];
	// ZwrÛcenie wyniku
	result2.x=result[0];
	result2.y=result[1];
	result2.z=result[2];
	return result2;
};

// FUNKCJA ZAMIENIA WSP”£RZ DNE EKRANOWE NA WSP”£RZ DNE SCENU

sPoint3D WinToGl(sPoint3D pix){
	GLdouble result[3];
	sPoint3D result2;
	GLdouble win_x,win_y,win_z;
	// Macierze przekszta≥ceÒ
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	// Pobranie macierzy przekszta≥ceÒ
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);
	// Modyfikacja obliczanego punktu o wysokoúÊ okna zwiπzana z tym iø oú Y w Windows biegnie od gÛrnego lewego rogu w dÛ≥
	pix.y=WINDOW_HEIGHT-pix.y;
	// Obliczenie wyniku
	gluUnProject(pix.x,pix.y,pix.z,modelview,projection,viewport,&result[0],&result[1],&result[2]);
	// ZwrÛcenie wyniku
	result2.x=result[0];
	result2.y=result[1];
	result2.z=result[2];
	return result2;
}

// ZAMIENIA STOPNIE NA RADIANY

long double DegToRad(long double x){
	return (x*PI)/180.0;
};

//------------------------------------------------
//               METODY KLASY cClock
//------------------------------------------------

long double cClock::julian_day() {
	long double JD,L,M,G,T,x, new_year;
	//	Jeúli data jest przed naszπ erπ to zamieÒ wartoúÊ roku na liczbÍ ujemnπ
	if (bc) {
		new_year = year*(-1) + 1;
	} else {
		new_year = year;
	}
	// Wzory implementujπce wprost oryginalny algorytm Hatchera
	x = (14 - month)/12;
	L = new_year + 4716;
    L -=floor(x);
	M = (month + 9) % 12;
	x=(L + 184)/100;
	x=floor(x);
	x=3*x/4;
	G = floor(x) - 38;
	x = 365.25*L;
	JD = floor(x);
	x=30.6*M+0.4;
	JD += floor(x);
	// Przeliczenie godziny na u≥amek doby
	T= hour/24.0 + minute/(60.0 * 24.0) + second/(3600.0 * 24.0);
	// Ostateczny wynik
	JD += day - G - 1402 + T;
	return JD;
};

void cClock::inc_time(long double msec) {
	long double G, J, I, JD, x;
	JD = julian_day();
	millisecond = millisecond + msec;
	// Poniøszy ciπg warunkÛw pozwala na sprawdzenie przepe≥nienia poszczegÛlnych sk≥adowych godziny jeúli msec jest bardzo duøe
	if (millisecond<0) {
		second = second - (abs(millisecond)+1000)/1000; 
		millisecond = 1000*(abs(millisecond)/1000+1)-abs(millisecond);
	} else {
		second = second + millisecond/1000;
		millisecond = millisecond % 1000;
	}
	if (second<0) {
		minute = minute - (abs(second)+60)/60; 
		second = 60*(abs(second)/60+1)-abs(second);
	} else {
		minute = minute + second/60;
		second = second % 60;
	}
	if (minute<0) {
		hour = hour - (abs(minute)+60)/60; 
		minute = 60*(abs(minute)/60+1)-abs(minute);
	} else {
		hour = hour + minute/60;
		minute = minute % 60;
	}
	if (hour<0) {
		JD = JD - (abs(hour)+24)/24; 
		hour = 24*(abs(hour)/24+1)-abs(hour);
	} else {
		JD = JD + hour/24;
		hour = hour % 24;
	}
	JD = floor(JD);
	//ponowna konwersja na datÍ kalendarza gregoriaÒskiego
	x = JD + 68569;
	x = x/36524.25;
	x = floor(x);
	x = (3*x)/4.0;
	x = floor(x);
	G = x - 38;
	J = 4*(JD + G + 1401) + 3;
	int intJ=J;
	x = intJ % 1461;
	x = x/4.0;
	x = floor(x);
	I = 5*x + 2;
	int intI=I;
	x = intI % 153;
	x = x/5.0;
	x = floor(x);
	day = x + 1;
	x = I/153 + 2;
	x = floor(x);
	int intx=x;
	month = (intx % 12) + 1;
	x = J/1461;
	x = floor(x);
	year = x - 4716;
	x = (long double)(14 -  month)/12.0;
	x = floor(x);
	year = year + x;
	// Ustawienie odpowiedni flagi okreúlajπcej erÍ i ewentalne modyfikacja roku jeúli jest liczbπ ujemnπ
	if (year<=0) {
		year=year*(-1) + 1;
		bc=true;
	} else bc = false;
};

void cClock::set_current_time() {
	SYSTEMTIME st;
	//Pobiera czas systemowy
	GetLocalTime(&st);
	year=st.wYear;
	month=st.wMonth;
	day=st.wDay;
	hour=st.wHour;
	minute=st.wMinute;
	second=st.wSecond;
	millisecond=st.wMilliseconds;
	bc=false;
};

//------------------------------------------------
//               METODY KLASY cCamera
//------------------------------------------------

void cCamera::change_focus(long double x, long double y, long double z, long double r){
	// Ustawiamy focus na øπdanπ wartoúÊ
	set_focus(x,y,z,r);
	sPoint3D move_vector;
	double long move_radius;
	// Zmniejszamy zmiennπ sterujπcπ interpolacjπ o 1
	focus_interpolation--;
	// Interpolujemy odleg≥oúÊ miÍdzy starym punktem centralnym a nowym
	move_vector.x=(focus.x-old_focus.x)*((30-focus_interpolation)/30.0);
	move_vector.y=(focus.y-old_focus.y)*((30-focus_interpolation)/30.0);
	move_vector.z=(focus.z-old_focus.z)*((30-focus_interpolation)/30.0);
	// Interpolujemy minimalny promieÒ
	move_radius=(inf_radius-old_inf_radius)*((30-focus_interpolation)/30.0);
	// Ustawiamy punkt widzenia kamery na obliczone wartoúci
	set_focus(old_focus.x+move_vector.x,old_focus.y+move_vector.y,old_focus.z+move_vector.z,old_inf_radius+move_radius);
};

void cCamera::set_focus(long double x, long double y, long double z, long double r){
	focus.x=x;
	focus.y=y;
	focus.z=z;
	inf_radius=r;
};

//------------------------------------------------
//               METODY KLASY cPlanet
//------------------------------------------------

bool cPlanet::generate_orbit_line() {
	// Przejdü po wszytskich punktach tworzπcych liniÍ orbity
	for (int i=0; i < ORBIT_PRECISION; i++) {
		// Oblicz i-ty punkt
		calculate_sun_positions(timer.julian_day()+sun_year/ORBIT_PRECISION*i);
		orbit_line[i] = get_sun_position();
	}
	return true;
};

bool cPlanet::calculate_sun_positions(long double jd) {
	double long a, e, I, L, W, O;
	double long T,w,M,e_deg,E,delta_M,delta_E,x,y;
	T=(jd-2451545.0)/36525.0;
	a=orbit.a+T*orbit.a_pc;
	e=orbit.e+T*orbit.e_pc;
	I=orbit.I+T*orbit.I_pc;
	L=orbit.L+T*orbit.L_pc;
	W=orbit.W+T*orbit.W_pc;
	O=orbit.O+T*orbit.O_pc;
	w=W-O;
	M=L-W+orbit.b*T*T+orbit.c*cos(DegToRad(orbit.f*T))+orbit.s*sin(DegToRad(orbit.f*T));
	while (M>180) M-=360;
	while (M<-180) M+=360;
	e_deg=(180.0/PI)*e;
	E=M+e_deg*sin(DegToRad(M));
	do {
		delta_M=M-(E-e_deg*sin(DegToRad(E)));
		delta_E=delta_M/(1-e*cos(DegToRad(E)));
		E=E+delta_E;
	} while (delta_E>0.000001);
	x=a*(cos(DegToRad(E))-e);
	y=a*sqrt(1-e*e)*sin(DegToRad(E));
	sun_position.x=(cos(DegToRad(w))*cos(DegToRad(O))-sin(DegToRad(w))*sin(DegToRad(O))*cos(DegToRad(I)))*x+(-sin(DegToRad(w))*cos(DegToRad(O))-cos(DegToRad(w))*sin(DegToRad(O))*cos(DegToRad(I)))*y;
	sun_position.y=(cos(DegToRad(w))*sin(DegToRad(O))+sin(DegToRad(w))*cos(DegToRad(O))*cos(DegToRad(I)))*x+(-sin(DegToRad(w))*sin(DegToRad(O))+cos(DegToRad(w))*cos(DegToRad(O))*cos(DegToRad(I)))*y;
	sun_position.z=(sin(DegToRad(w))*sin(DegToRad(I)))*x+(cos(DegToRad(w))*sin(DegToRad(I)))*y;
	return true;
};

bool cPlanet::calculate_screen_position(sPoint3D move){
	sun_position.x-=move.x;
	sun_position.y-=move.y;
	sun_position.z-=move.z;
	screen_position = GlToWin(sun_position);
	//Zaokrπglamy zmiennπ z
	screen_position.z = floor(screen_position.z);
	return true;
};

long double cPlanet::get_axial_tilt(){
	return axial_tilt;
};

long double cPlanet::get_diameter(){
	return diameter;
};

sPoint3D cPlanet::get_orbit_line(int num){
	// Jeúli chcemy uzyskaÊ element, ktÛrego nie ma w tablicy
	if (num<0 || num>=ORBIT_PRECISION) {
		// ZwruÊ zero
		sPoint3D result = {0,0,0};
		return result;
	} else return orbit_line[num];
};

long double cPlanet::get_rotation_speed(){
	return rotation_speed;
};

sPoint3D cPlanet::get_screen_position(){
	return screen_position;
};

sPoint3D cPlanet::get_sun_position(){
	return sun_position;
};

long double cPlanet::get_sun_year(){
	return sun_year;
};

bool cPlanet::set_axial_tilt(long double new_axial_tilt){
	axial_tilt=new_axial_tilt;
	return true;
};

bool cPlanet::set_diameter(long double new_diameter){
	diameter=new_diameter;
	return true;
};

bool cPlanet::set_orbit(double long a,double long a_pc,double long b,double long c,double long e,double long e_pc,double long f,double long I,double long I_pc,double long L,double long L_pc,double long O,double long O_pc,double long s,double long W,double long W_pc){
	orbit.a = a;
	orbit.a_pc = a_pc;
	orbit.b = b;
	orbit.c = c;
	orbit.e = e;
	orbit.e_pc = e_pc;
	orbit.f = f;
	orbit.I = I;
	orbit.I_pc = I_pc;
	orbit.L = L;
	orbit.L_pc = L_pc;
	orbit.O = O;
	orbit.O_pc = O_pc;
	orbit.s = s;
	orbit.W = W;
	orbit.W_pc = W_pc;
	return true;
};

bool cPlanet::set_rotation_speed(long double new_rotation_speed){
	rotation_speed=new_rotation_speed;
	return true;
};

bool cPlanet::set_sun_year(long double new_sun_year){
	sun_year=new_sun_year;
	return true;
};

//------------------------------------------------
//               METODY KLASY cBMP
//------------------------------------------------

cBMP::cBMP(){
	// Øadna bitmapa nie zosta≥a jeszcze wczytana
	isLoaded=false;
};

int cBMP::GetHeight(){
	return txBitmap.Height;
};

int cBMP::GetWidth(){
	return txBitmap.Width;
};

GLuint cBMP::GetID(){
	return txBitmap.TextureID;
};

void cBMP::LoadBMP(char* filename, bool resize, int new_width, int new_height){
	//Ustaw wysokπ jakoúÊ tekstÛr
	Bitmap.SetHighQualityTextures(true);
	//Wy≥πcz filtrowania
	Bitmap.SetTextureFilter(txNoFilter);
	//Ustaw kolor przezroczysty
	Bitmap.SetAlphaMatch(true,255,0,255);
	//Za≥aduj bitmapÍ z dysku
	Bitmap.LoadTextureFromDisk(filename,&txBitmap);
	//Jeúli flaga skalowania jest w≥πczona to ustaw rozmiar podany w parametrach
	if (resize) {
		width=new_width;
		height=new_height;
	} else {
		//Jeúli nie to ustaw rzeczywisty rozmiar bitmapy
		width=txBitmap.Width;
		height=txBitmap.Height;
	}
	isLoaded=true;
};

cBMP::~cBMP(){
	// Jeúli zosta≥a wczytana bitmapa to jπ usuÒ
	if (isLoaded) Bitmap.FreeTexture(&txBitmap);
};

void cBMP::PrintBMP(int x_pos, int y_pos){
	sPoint3D rect[4];
	sPoint3D help;
	// Oblicza wspÛ≥rzÍdne wszytstkich 4 rogÛw bitmapy
	// Lewy GÛrny
	help.x = x_pos;
	help.y = y_pos;
	help.z = 0;
	rect[0] = WinToGl(help);
	// Prawy GÛrny
	help.x = x_pos+width;
	help.y = y_pos;
	help.z = 0;
	rect[1] = WinToGl(help);
	// Prawy Dolny
	help.x = x_pos;
	help.y = y_pos+height;
	help.z = 0;
	rect[2] = WinToGl(help);
	// Lewy Dolny
	help.x = x_pos+width;
	help.y = y_pos+height;
	help.z = 0;
	rect[3] = WinToGl(help);
	//Ustaw bierzπcπ teksturÍ na wyúwietlany obrazek
	glBindTexture(GL_TEXTURE_2D,txBitmap.TextureID);
	//Ustaw mieszanie kolorÛw tak by nie wyúwietlaÊ koloru przezroczystego
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//Wyúwietlenie obrazka
	glBegin(GL_QUADS);
		glTexCoord2f(0,1);
		glVertex3f(rect[0].x,rect[0].y,rect[0].z);
		glTexCoord2f(1,1);
		glVertex3f(rect[1].x,rect[1].y,rect[1].z);
		glTexCoord2f(1,0);
		glVertex3f(rect[3].x,rect[3].y,rect[3].z);
		glTexCoord2f(0,0);
		glVertex3f(rect[2].x,rect[2].y,rect[2].z);
	glEnd();
	//Wy≥πcz mieszanie kolorÛw
	glDisable(GL_BLEND);
};

//------------------------------------------------
//               METODY KLASY cButton
//------------------------------------------------

void cButton::InitButton(RECT rectangle,cBMP* active,cBMP* not_active,cBMP* pressed) {
	ButtonActive = active;
	ButtonNotActive = not_active;
	ButtonPressed = pressed;
	rcArea = rectangle;
};

bool cButton::IsInArea(int x,int y) {
	if (x>rcArea.left && x<rcArea.right && y>rcArea.top && y<rcArea.bottom) return true; else return false;
};

void cButton::Disable(){
	bActive = false;
};

void cButton::Enable(){
	bActive = true;
};

bool cButton::IsActive(){
	return bActive;
};

void cButton::PrintButton(bool pressed, int x_pos, int y_pos) {
	// Jeúli guzik jest naciskany
	if (pressed) {
		//Wyúwietl go jako naciskany
		ButtonPressed->PrintBMP(x_pos,y_pos);
	// Jeúli nie to wyúwietl go zgodnie z jego stanem
	} else if(IsActive()) {
		ButtonActive->PrintBMP(x_pos,y_pos);
	} else {
		ButtonNotActive->PrintBMP(x_pos,y_pos);
	}
};

// PROCEDURA ZDARZENIOWA OKNA PROGRAMU

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		// Jeúli okno jest minimalizowane lub przywracane to ustaw odpwoiedniπ flagÍ
		case WM_ACTIVATE:{	
			if(!HIWORD(wParam)){
				active=TRUE;
			} else {
				active=FALSE;						
			}
			return 0;								
		}
		// Jeúli system prÛbuje w≥πczyÊ wygaszacz albo wy≥πczyÊ monitor
		case WM_SYSCOMMAND:{ 
			switch(wParam){
				case SC_SCREENSAVE:					
				case SC_MONITORPOWER:				
				return 0;							
			}
			break;									
		}
		// Gdy jest zamykane okno
		case WM_CLOSE:{
			PostQuitMessage(0);						
			return 0;								
		}
		// Jeúli zosta≥ wciúniÍrty klawisz to ustaw odpowiedniπ flagÍ
		case WM_KEYDOWN:{
			keys[wParam] = TRUE;					
			return 0;								
		}
		// A jeúli zosta≥ zwolniony to usuÒ flagÍ
		case WM_KEYUP:{
			keys[wParam] = FALSE;					
			return 0;								
		}
		// Jeúli zosta≥ wykonany ruch rolkπ myszki
		case WM_MOUSEWHEEL:{
			//odczytaj odchylenie rolki
			mouse_delta = (short) HIWORD(wParam);
			// ustaw zmiennπ mouse_roll zgodnie z odchyleniem rolki
			if (mouse_delta==120) mouse_roll=1;
			if (mouse_delta==-120) mouse_roll=-1;
			return 0;
		}
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
};

// FUNKCJA INICJUJE BIBLIOTEKI (OpenGL i DirectSound), WCZYTUJE PLIKI GRAFICZNE I DèWI KOWE, I DOKONUJE WCZYTANIA DOMYåLNYCH WARTOåCI DO WSZYTSKICH ZMIENNYCH

void Init() {
	GLuint      PixelFormat;			  // Przechowuje wynik wyszukiwania odpowiedniego formatu
	WNDCLASS	wc;						  // Struktura klasy okna
	DWORD		dwExStyle;				  // Rozszerzony styl okna
	DWORD		dwStyle;				  // Styl okna
	RECT		WindowRect;				  // Obszar roboczy okna
	
	// Ustaw obszar roboczy okna
	WindowRect.left=(long)0;			   
	WindowRect.right=(long)WINDOW_WIDTH;   
	WindowRect.top=(long)0;				  
	WindowRect.bottom=(long)WINDOW_HEIGHT;
	
	// Definiujemy klasÍ okna
	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	
	wc.lpfnWndProc = (WNDPROC) WndProc;					
	wc.cbClsExtra = 0;									
	wc.cbWndExtra = 0;									
	wc.hInstance = hInstance;							
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			
	wc.hbrBackground = NULL;									
	wc.lpszMenuName = NULL;									
	wc.lpszClassName = "Orrery";								

	RegisterClass(&wc);  // Rejestrujemy klase okna w systemie
	
	// Definiujemy tryb okreanu
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = WINDOW_WIDTH;
	dmScreenSettings.dmPelsHeight = WINDOW_HEIGHT;
	dmScreenSettings.dmBitsPerPel = COLOR_DEPTH;
	dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

	ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN); //Ustawiamy zdefiniowany tryb ekranu
	
	// Ustawiamy style okna
	dwExStyle = WS_EX_APPWINDOW;
	dwStyle = WS_POPUP;

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Tworzymy okno
	hWnd = CreateWindowEx(	dwExStyle,
							"Orrery",							
							"The Orrery",						
							dwStyle |							
							WS_CLIPSIBLINGS |					
							WS_CLIPCHILDREN,					
							0, 0,								
							WindowRect.right-WindowRect.left,	
							WindowRect.bottom-WindowRect.top,	
							NULL,								
							NULL,								
							hInstance,							
							NULL);					     		
	
	// Definiujemy format wyúwietlania (dla OpenGL)
	static	PIXELFORMATDESCRIPTOR pfd =	{			
		sizeof(PIXELFORMATDESCRIPTOR),				
		1,											
		PFD_DRAW_TO_WINDOW |						
		PFD_SUPPORT_OPENGL |					
		PFD_DOUBLEBUFFER,					
		PFD_TYPE_RGBA,							
		COLOR_DEPTH,					
		0, 0, 0, 0, 0, 0,				
		0,									
		0,									
		0,										
		0, 0, 0, 0,								
		16,											
		0,											
		0,											
		PFD_MAIN_PLANE,								
		0,											
		0, 0, 0										
	};
	
	// Zapisujemy uchwyt do kontekstu urzπdzenia utworzonego okna
	hDC = GetDC(hWnd);			
	
	// Wybieramy i ustawiamy odpowiedni format wyúwietlania
	PixelFormat=ChoosePixelFormat(hDC,&pfd);
	SetPixelFormat(hDC,PixelFormat,&pfd);
	
	//Tworzymy kontekst renderowania i aktywujemy go
	hRC=wglCreateContext(hDC);		
	wglMakeCurrent(hDC,hRC);

	//Czynimy utworzone okno widocznym i ustawiamy je tak by znajdowa≥o siÍ na wierzchu
	ShowWindow(hWnd,SW_SHOW);					
	SetForegroundWindow(hWnd);					
	SetFocus(hWnd);									
	
	//Ustawiamy obszar wyúwietlania dla OpenGL
	glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
	
	//Zerujemy macierz projekcji
	glMatrixMode(GL_PROJECTION);					
	glLoadIdentity();							
	
	//Ustawiamy perspektywÍ
	gluPerspective(45.0f,(GLfloat)WINDOW_WIDTH/(GLfloat)WINDOW_HEIGHT,0.1f,1000.0f);
	
	//Zerujemy macierz modelu
	glMatrixMode(GL_MODELVIEW);					
	glLoadIdentity();							
	
	//£adujemy elementy graficzne zegara

	//era
	Interface.ClockBC[0].LoadBMP("GFX/txt_ne.bmp",true,30,10);
	Interface.ClockBC[1].LoadBMP("GFX/txt_pne.bmp",true,30,10);
	//miesiπce
	for (int i=0; i<12; i++) {
		std::stringstream fname;
		fname << "GFX/txt_month_" << i << ".bmp";
		char string[256];
		strcpy(string,fname.str().c_str());
		Interface.ClockMonths[i].LoadBMP(string,true,65,10);
	}
	//cyfry
	for (int i=0; i<10; i++) {
		std::stringstream fname;
		fname << "GFX/txt_" << i << ".bmp";
		char string[256];
		strcpy(string,fname.str().c_str());
		Interface.ClockDigits[i].LoadBMP(string,true,6,7);
	}
	
	//£adujemy elementy graficzne interface'u
	Interface.Panel.LoadBMP("GFX/bar.bmp",true,709,22);
	Interface.ClockHighlightGFX[CH_AGE].LoadBMP("GFX/mark_age.bmp",true,34,14);
	Interface.ClockHighlightGFX[CH_YEAR].LoadBMP("GFX/mark_year.bmp",true,34,14);
	Interface.ClockHighlightGFX[CH_MONTH].LoadBMP("GFX/mark_month.bmp",true,69,14);
	Interface.ClockHighlightGFX[CH_DAY].LoadBMP("GFX/mark_number.bmp",true,18,14);
	Interface.ClockHighlightGFX[4].LoadBMP("GFX/empty.bmp",false,0,0);
	Interface.ButtonsGFX[BT_EXIT][0].LoadBMP("GFX/button_exit.bmp",true,19,19);
	Interface.ButtonsGFX[BT_INFO][0].LoadBMP("GFX/button_info.bmp",true,19,19);
	Interface.ButtonsGFX[BT_MUSIC][0].LoadBMP("GFX/button_music_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_MUSIC][1].LoadBMP("GFX/button_music_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_MUSIC][2].LoadBMP("GFX/button_music_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_SOUND][0].LoadBMP("GFX/button_sound_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_SOUND][1].LoadBMP("GFX/button_sound_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_SOUND][2].LoadBMP("GFX/button_sound_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_ORBIT][0].LoadBMP("GFX/button_orbit_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_ORBIT][1].LoadBMP("GFX/button_orbit_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_ORBIT][2].LoadBMP("GFX/button_orbit_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_LABELS][0].LoadBMP("GFX/button_labels_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_LABELS][1].LoadBMP("GFX/button_labels_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_LABELS][2].LoadBMP("GFX/button_labels_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_BIRDVIEW][0].LoadBMP("GFX/button_birdview_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_BIRDVIEW][1].LoadBMP("GFX/button_birdview_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_BIRDVIEW][2].LoadBMP("GFX/button_birdview_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_BCKG][0].LoadBMP("GFX/button_stars_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_BCKG][1].LoadBMP("GFX/button_stars_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_BCKG][2].LoadBMP("GFX/button_stars_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_PAUSE][0].LoadBMP("GFX/button_pause_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_PAUSE][1].LoadBMP("GFX/button_pause_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_PAUSE][2].LoadBMP("GFX/button_pause_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_SLIDE][0].LoadBMP("GFX/button_slide.bmp",true,24,41);
	Interface.ButtonsGFX[BT_RESET_SPEED][0].LoadBMP("GFX/button_realtime_on.bmp",true,19,19);
	Interface.ButtonsGFX[BT_RESET_SPEED][1].LoadBMP("GFX/button_realtime_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_RESET_SPEED][2].LoadBMP("GFX/button_realtime_high.bmp",true,19,19);
	Interface.ButtonsGFX[BT_RESET_TIME][1].LoadBMP("GFX/button_reset_off.bmp",true,19,19);
	Interface.ButtonsGFX[BT_RESET_TIME][2].LoadBMP("GFX/button_reset_high.bmp",true,19,19);
	
	//Inicjujemy przyciski

	RECT aux_rect; //Zmienna pomocnicza
	
	aux_rect.top = 751;
	//Element zegara: era
	aux_rect.left = 626;
	aux_rect.right = aux_rect.left + 34;
	aux_rect.bottom = aux_rect.top + 14;
	Interface.ClockHighlight[CH_AGE].InitButton(aux_rect,&Interface.ClockHighlightGFX[CH_AGE],&Interface.ClockHighlightGFX[4],&Interface.ClockHighlightGFX[CH_AGE]);
	//Element zegara: rok
	aux_rect.left = 661;
	aux_rect.right = aux_rect.left + 34;
	aux_rect.bottom = aux_rect.top + 14;
	Interface.ClockHighlight[CH_YEAR].InitButton(aux_rect,&Interface.ClockHighlightGFX[CH_YEAR],&Interface.ClockHighlightGFX[4],&Interface.ClockHighlightGFX[CH_YEAR]);
	//Element zegara: miesiπc
	aux_rect.left = 696;
	aux_rect.right = aux_rect.left + 69;
	aux_rect.bottom = aux_rect.top + 14;
	Interface.ClockHighlight[CH_MONTH].InitButton(aux_rect,&Interface.ClockHighlightGFX[CH_MONTH],&Interface.ClockHighlightGFX[4],&Interface.ClockHighlightGFX[CH_MONTH]);
	//Element zegara: dzieÒ
	aux_rect.left = 766;
	aux_rect.right = aux_rect.left + 18;
	aux_rect.bottom = aux_rect.top + 14;
	Interface.ClockHighlight[CH_DAY].InitButton(aux_rect,&Interface.ClockHighlightGFX[CH_DAY],&Interface.ClockHighlightGFX[4],&Interface.ClockHighlightGFX[CH_DAY]);
	//Element zegara: godzina
	aux_rect.left = 802;
	aux_rect.right = aux_rect.left + 18;
	aux_rect.bottom = aux_rect.top + 14;
	Interface.ClockHighlight[CH_HOUR].InitButton(aux_rect,&Interface.ClockHighlightGFX[CH_DAY],&Interface.ClockHighlightGFX[4],&Interface.ClockHighlightGFX[CH_DAY]);
	//Element zegara: minuta
	aux_rect.left = 821;
	aux_rect.right = aux_rect.left + 18;
	aux_rect.bottom = aux_rect.top + 14;
	Interface.ClockHighlight[CH_MINUTE].InitButton(aux_rect,&Interface.ClockHighlightGFX[CH_DAY],&Interface.ClockHighlightGFX[4],&Interface.ClockHighlightGFX[CH_DAY]);
	//Element zegara: sekunda
	aux_rect.left = 840;
	aux_rect.right = aux_rect.left + 18;
	aux_rect.bottom = aux_rect.top + 14;
	Interface.ClockHighlight[CH_SECONDS].InitButton(aux_rect,&Interface.ClockHighlightGFX[CH_DAY],&Interface.ClockHighlightGFX[4],&Interface.ClockHighlightGFX[CH_DAY]);
	//Wyjúcie z programu 
	aux_rect.left = 1003;
	aux_rect.top = 2;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_EXIT].InitButton(aux_rect,&Interface.ButtonsGFX[BT_EXIT][0],&Interface.ButtonsGFX[BT_EXIT][0],&Interface.ButtonsGFX[BT_EXIT][0]);
	//Guzik Informacji
	aux_rect.left = 5;
	aux_rect.top = 5;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_INFO].InitButton(aux_rect,&Interface.ButtonsGFX[BT_INFO][0],&Interface.ButtonsGFX[BT_INFO][0],&Interface.ButtonsGFX[BT_INFO][0]);
	//Muzyka
	aux_rect.left = 162;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_MUSIC].InitButton(aux_rect,&Interface.ButtonsGFX[BT_MUSIC][0],&Interface.ButtonsGFX[BT_MUSIC][1],&Interface.ButtonsGFX[BT_MUSIC][2]);
	Interface.Buttons[BT_MUSIC].Enable();
	//DüwiÍk
	aux_rect.left = 182;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_SOUND].InitButton(aux_rect,&Interface.ButtonsGFX[BT_SOUND][0],&Interface.ButtonsGFX[BT_SOUND][1],&Interface.ButtonsGFX[BT_SOUND][2]);
	Interface.Buttons[BT_SOUND].Enable();
	//Orbita
	aux_rect.left = 206;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_ORBIT].InitButton(aux_rect,&Interface.ButtonsGFX[BT_ORBIT][0],&Interface.ButtonsGFX[BT_ORBIT][1],&Interface.ButtonsGFX[BT_ORBIT][2]);
	Interface.Buttons[BT_ORBIT].Enable();
	//Etykietki planet
	aux_rect.left = 226;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_LABELS].InitButton(aux_rect,&Interface.ButtonsGFX[BT_LABELS][0],&Interface.ButtonsGFX[BT_LABELS][1],&Interface.ButtonsGFX[BT_LABELS][2]);
	Interface.Buttons[BT_LABELS].Enable();
	//Widok z gÛry
	aux_rect.left = 246;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_BIRDVIEW].InitButton(aux_rect,&Interface.ButtonsGFX[BT_BIRDVIEW][0],&Interface.ButtonsGFX[BT_BIRDVIEW][1],&Interface.ButtonsGFX[BT_BIRDVIEW][2]);
	//T≥o
	aux_rect.left = 266;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_BCKG].InitButton(aux_rect,&Interface.ButtonsGFX[BT_BCKG][0],&Interface.ButtonsGFX[BT_BCKG][1],&Interface.ButtonsGFX[BT_BCKG][2]);
	Interface.Buttons[BT_BCKG].Enable();
	//Pauza
	aux_rect.left = 290;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_PAUSE].InitButton(aux_rect,&Interface.ButtonsGFX[BT_PAUSE][0],&Interface.ButtonsGFX[BT_PAUSE][1],&Interface.ButtonsGFX[BT_PAUSE][2]);
	//Suwak
	aux_rect.left = 450+time_speed-10;
	aux_rect.top = 726;
	aux_rect.right = aux_rect.left + 24;
	aux_rect.bottom = aux_rect.top + 41;
	Interface.Buttons[BT_SLIDE].InitButton(aux_rect,&Interface.ButtonsGFX[BT_SLIDE][0],&Interface.ButtonsGFX[BT_SLIDE][0],&Interface.ButtonsGFX[BT_SLIDE][0]);
	//Zeruj prÍdkoúÊ
	aux_rect.left = 596;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_RESET_SPEED].InitButton(aux_rect,&Interface.ButtonsGFX[BT_RESET_SPEED][0],&Interface.ButtonsGFX[BT_RESET_SPEED][1],&Interface.ButtonsGFX[BT_RESET_SPEED][2]);
	//ustaw czas systemowy
	aux_rect.left = 870;
	aux_rect.top = 748;
	aux_rect.right = aux_rect.left + 19;
	aux_rect.bottom = aux_rect.top + 19;
	Interface.Buttons[BT_RESET_TIME].InitButton(aux_rect,&Interface.ButtonsGFX[BT_RESET_TIME][1],&Interface.ButtonsGFX[BT_RESET_TIME][1],&Interface.ButtonsGFX[BT_RESET_TIME][2]);
	
	//£aduj tekstury planet
	planet_textures[0].LoadBMP("GFX/texture_sun.jpg",false,0,0);
	planet_textures[1].LoadBMP("GFX/texture_mercury.jpg",false,0,0);
	planet_textures[2].LoadBMP("GFX/texture_venus.jpg",false,0,0);
	planet_textures[3].LoadBMP("GFX/texture_earth.jpg",false,0,0);
	planet_textures[4].LoadBMP("GFX/texture_mars.jpg",false,0,0);
	planet_textures[5].LoadBMP("GFX/texture_jupiter.jpg",false,0,0);
	planet_textures[6].LoadBMP("GFX/texture_saturn.jpg",false,0,0);
	planet_textures[7].LoadBMP("GFX/texture_uranus.jpg",false,0,0);
	planet_textures[8].LoadBMP("GFX/texture_neptune.jpg",false,0,0);
	planet_textures[9].LoadBMP("GFX/texture_pluto.jpg",false,0,0);
	
	//Ustawiamy parametry wyúwietlania
	glEnable(GL_TEXTURE_2D);							
	glShadeModel(GL_SMOOTH);							
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				
	glClearDepth(1.0f);									
	glEnable(GL_DEPTH_TEST);							
	glDepthFunc(GL_LEQUAL);								
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	//Inicjujemy wartoúci kamery 
	camera.radius=10;
	camera.alpha=45;
	camera.theta=0;
	camera.set_focus(0,0,0,0.9-planet[selected].get_diameter());
	camera.focus_interpolation=0;
	
	//Ustawiamy úrednicÍ s≥oÒca
	planet[0].set_diameter(0.2);
	
	//Inicjujemy planety

	//Merkury
	planet[1].set_orbit(0.38709843,0,0,0,0.20563661,0.00002123,0,7.00559432,-0.00590158,252.25166724,149472.67486623,48.33961819,-0.12214182,0,77.45771895,0.15940013);
	planet[1].set_diameter(0.097822);
	planet[1].set_rotation_speed(58.6462);
	planet[1].set_axial_tilt(0.01);
	planet[1].set_sun_year(87.969);
	//Wenus
	planet[2].set_orbit(0.72332102,-0.00000026,0,0,0.00676399,-0.00005107,0,3.39777545,0.00043494,181.97970850,58517.81560260,76.67261496,-0.27274174,0,131.76755713,0.05679648);
	planet[2].set_diameter(0.2427266);
	planet[2].set_rotation_speed(-243.0185);
	planet[2].set_axial_tilt(2.64);
	planet[2].set_sun_year(224.7);
	//Ziemia
	planet[3].set_orbit(1.00000018,-0.00000003,0,0,0.01673163,-0.00003661,0,-0.00054346,-0.01337178,100.46691572,35999.37306329,-5.11260389,-0.24123856,0,102.93005885,0.31795260);
	planet[3].set_diameter(0.2558);
	planet[3].set_rotation_speed(1);
	planet[3].set_axial_tilt(23.439281);
	planet[3].set_sun_year(365.25696);
	//Mars
	planet[4].set_orbit(1.52371243,0.00000097,0,0,0.09336511,0.00009149,0,1.85181869,-0.00724757,-4.56813164,19140.29934243,49.71320984,-0.26852431,0,-23.91744784,0.45223625);
	planet[4].set_diameter(0.1364658);
	planet[4].set_rotation_speed(1.025957);
	planet[4].set_axial_tilt(25.19);
	planet[4].set_sun_year(686.96);
	//Jowisz
	planet[5].set_orbit(5.20248019,-0.00002864,-0.0012452,0.06064060,0.04853590,0.00018026,38.35125,1.29861416,-0.00322699,34.33479152,3034.90371757,100.29282654,0.13024619,-0.35635438,14.27495244,0.18199196);
	planet[5].set_diameter(2.86736);
	planet[5].set_rotation_speed(0.4135);
	planet[5].set_axial_tilt(3.13);
	planet[5].set_sun_year(4332.589);
	//Saturn
	planet[6].set_orbit(9.54149883,-0.00003065,0.0025899,-0.13434469,0.05550825,-0.0032044,38.35125,2.49424102,0.00451969,50.07571329,1222.11494724,113.63998702,-0.25015002,0.87320147,92.86136063,0.54179478);
	planet[6].set_diameter(2.41719);
	planet[6].set_rotation_speed(0.449375);
	planet[6].set_axial_tilt(26.73);
	planet[6].set_sun_year(10756.1995);
	//Uran
	planet[7].set_orbit(19.18797948,-0.00020455,0.0058331,-0.97731848,0.04685740,-0.00001550,7.67025,0.77298127,-0.00180155,314.20276625,428.49512595,73.96250215,0.05739699,0.17689245,172.43404441,0.09266985);
	planet[7].set_diameter(1.02510);
	planet[7].set_rotation_speed(-0.71833);
	planet[7].set_axial_tilt(97.77);
	planet[7].set_sun_year(30707.4896);
	//Neptun
	planet[8].set_orbit(30.06952752,0.00006447,-0.00041348,0.68346317,0.00895439,0.00000818,7.67025,1.77005520,0.00022400,304.22289287,218.46515314,131.78635853,-0.00606302,-0.10162547,46.68158724,0.01009938);
	planet[8].set_diameter(0.993222);
	planet[8].set_rotation_speed(0.67125);
	planet[8].set_axial_tilt(28.32);
	planet[8].set_sun_year(60223.3528);
	//Pluton
	planet[9].set_orbit(39.48686035,0.00449751,-0.01262724,0,0.24885238,0.00006016,0,17.14104260,0.00000501,238.96535011,145.18042903,110.30167986,-0.00809981,0,224.09702598,-0.00968827);
	planet[9].set_diameter(0.047928);
	planet[9].set_rotation_speed(-6.387230);
	planet[9].set_axial_tilt(119.591);
	planet[9].set_sun_year(90613.3055);
	
	//£asujemy bitmapÍ z pierúcieniami saturna
	saturn_ring.LoadBMP("GFX/saturn_ring.bmp",false,0,0);
	
	//Ustawiamy wartoúÊ zegara na aktualny czas
	timer.set_current_time();
	
	//Ustawiamy aktualnπ planetÍ (czyli na tπ, wzglÍdej ktÛrej jest zorientowana scena) na 0 (czyli s≥oÒce)
	selected=0;
	
	//Obliczamy linie tworzπce orbity planet
	for (int i=1; i<10; i++) planet[i].generate_orbit_line();
	
	//Wczytujemy bitmapÍ tworzπcπ t≥o
	stars_bkg.LoadBMP("GFX/stars.jpg",false,0,0);
	
	//Wczytujemy okna z informacjami
	for (int i=0; i<10; i++) {
		std::stringstream fname;
		fname << "GFX/info_" << i << ".bmp";
		char string[256];
		strcpy(string,fname.str().c_str());
		Interface.InfoWindows[i].LoadBMP(string,true,235,383);
	}
	Interface.InfoWindows[10].LoadBMP("GFX/info_mini.bmp",true,39,40);
	
	//Wczytujemy etykietki planety
	for (int i=0; i<10; i++) {
		std::stringstream fname;
		fname << "GFX/planet_label_" << i << ".bmp";
		char string[256];
		strcpy(string,fname.str().c_str());
		Interface.LabelsGFX[i].LoadBMP(string,true,59,8);
	}
	
	//Wczytujemy etykietkÍ z tytu≥em programu
	title.LoadBMP("GFX/title.bmp",true,233,17);
	
	//Ustawiamy zmiennπ potrzebnπ do porawnej inicjaji düwiÍku
	main_window_handle=hWnd;
	
	//Inicjujemy DirectSound
	DSound_Init();

	//Wczytujemy etykieki düwiÍkowe planet
	for (int i=0; i<10; i++) {
		std::stringstream fname;
		fname << "AUDIO\\sound_label_" << i+1 << ".wav";
		char string[256];
		strcpy(string,fname.str().c_str());
		sound_labels[i]=DSound_Load_WAV(string);
	}
	
	//Wczytujemy pozosta≥e düwiÍki programu
	sound_button=DSound_Load_WAV("AUDIO\\button.wav");
	sound_timer_edit=DSound_Load_WAV("AUDIO\\time_edit.wav");
	sound_timer_edit_error=DSound_Load_WAV("AUDIO\\time_edit_error.wav");
	sound_slide=DSound_Load_WAV("AUDIO\\slide.wav");

	//Wczytujemy muzyke i zaczynamy jπ odtwarzaÊ
	music=DSound_Load_WAV("AUDIO\\music.wav");
	DSound_Play(music,1);
	DSound_Set_Volume(music,5);
	DSound_Set_Freq(music,44100);
};

//FUNKCJA PRZYGOTOWUJE PROGRAM DO ZAMKNI CIA

void Release() {
	//Zaprzestajemy odtwarzania düwiÍkÛw
	DSound_Delete_All_Sounds();
	//Wy≥πczamy DirectSound
	DSound_Shutdown();
	//Przywracamy domyúlne ustawienia monitora
	ChangeDisplaySettings(NULL,0);	
	//Usuwamy kontekst renderowania
	wglMakeCurrent(NULL,NULL);				
	wglDeleteContext(hRC);
	//Zwalniamy kontekst urzπdzenia okna
	ReleaseDC(hWnd,hDC);
	//Niszczymy okno
	DestroyWindow(hWnd);
	//Wyrejestrowujemy klase okna z systemu
	UnregisterClass("Orrery",hInstance);	
};

//FUNKCJA WYPISUJE PODAN• DODATNI• LICZB  CA£KOWITA NA EKRANIA
// value - wartoúÊ liczby, x_pos,y_pos - pozycja na ekrania, digits - liczba cyfr
void DrawNumber(int value, int x_pos, int y_pos, int digits){
	for (int i=0; i<digits; i++) {
		int digit=value%10;
		value=value/10;
		Interface.ClockDigits[digit].PrintBMP(x_pos+(digits-1-i)*8,y_pos);
	}
};

//FUNKCJA TWORZY I WYTåWIETLA PRZY POMOCY OPENGL OTEKSTUROWAN• KUL 
//c-pozycja, r-promieÒ, n-szczegÛ≥owoúÊ
//autor: Paul Bourke

void CreateSphere(sPoint3D c,double r,int n){
   int i,j;
   double theta1,theta2,theta3;
   sPoint3D e,p;
   if (r < 0) r = -r;
   if (n < 0) n = -n;
   if (n < 4 || r <= 0) {
      glBegin(GL_POINTS);
      glVertex3f(c.x,c.y,c.z);
      glEnd();
      return;
   }
   for (j=0;j<n/2;j++) {
      theta1 = j * TWOPI / n - PID2;
      theta2 = (j + 1) * TWOPI / n - PID2;

      glBegin(GL_QUAD_STRIP);
      for (i=0;i<=n;i++) {
         theta3 = i * TWOPI / n;

         e.x = cos(theta2) * cos(theta3);
         e.y = cos(theta2) * sin(theta3); 
         e.z = sin(theta2);
         p.x = c.x + r * e.x;
         p.y = c.y + r * e.y;
         p.z = c.z + r * e.z;

         glNormal3f(e.x,e.y,e.z);
         glTexCoord2f(i/(double)n,2*(j+1)/(double)n);
         glVertex3f(p.x,p.y,p.z);

         e.x = cos(theta1) * cos(theta3);
         e.y = cos(theta1) * sin(theta3);
         e.z = sin(theta1);
         p.x = c.x + r * e.x;
         p.y = c.y + r * e.y;
         p.z = c.z + r * e.z;

         glNormal3f(e.x,e.y,e.z);
         glTexCoord2f(i/(double)n,2*j/(double)n);
         glVertex3f(p.x,p.y,p.z);
      }
      glEnd();
   }
};

//FUNKCJA WYåWITLAJ•CA GRAFIK  NA EKRANIE

void DrawScene() {

	//Ustaw zmiennπ pomocniczπ wskazujπcπ úrodek uk≥adu s≥onecznego
	sPoint3D zero = {0,0,0};

	//Ustaw zmiennπ wskazujπcπ úrodek uk≥adu zmodyfikowanπ o wartoúci przesuniÍcia punktu na ktÛry patrzy kamera
	sPoint3D zero_moved;
	zero_moved.x=zero.x-camera.focus.x;
	zero_moved.y=zero.y-camera.focus.y;
	zero_moved.z=zero.z-camera.focus.z;

	//Zmienna pomconicza przy rysowaniu niektÛrych elementÛw interface'u
	RECT aux_rect;

	//Czyszczenie ekranu i zerowanie macierzy przekszta≥ceÒ
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();	

	//Ustawienie sceny wzglÍdem kamery
	//Zastosowa≥em tutaj pewnπ funkcjÍ wyk≥adniczπ od rzeczywistego promienia po to by przy duøych oddaleniach obraz oddala≥ siÍ szybciej niø przy ma≥ych
	glTranslatef(0,0,-pow((long double)10.0,(long double)camera.radius/50.0)+camera.inf_radius);
	glRotated(camera.alpha,1,0,0);
	glRotated(camera.theta,0,1,0);
	
	//Jeúli jest w≥πczone rysowanie t≥a to wyúwietl kulÍ na, ktÛrπ zostanie na≥oøona tekstura z t≥em
	if (Interface.Buttons[BT_BCKG].IsActive()) {
		glBindTexture(GL_TEXTURE_2D, stars_bkg.GetID());
		CreateSphere(zero,STARS_RADIUS,40);
	}

	// ObruÊ scenÍ o -90 stopni, by skorygowaÊ fakt, øe pozycje planet sπ prostopad≥e do p≥aszczyzny XZ 
	glRotated(-90,1,0,0);

	// Jesli jest ustawiona odpwoiednia opcja to yúwietl linie orbit planet
	if (Interface.Buttons[BT_ORBIT].IsActive()){
		for (int i=1; i<10; i++) {
			//Ustaw kolor linii na kolor pierwszego pixela tekstury danej planety (dzieki temu oribty bÍda psowaÊ kolorystycznie do planet)
			glBindTexture(GL_TEXTURE_2D, planet_textures[i].GetID());
			//Wyúwetl ≥amanπ ≥πczπcπ kolejno wszystkie punkty tworzπce liniÍ orbity
			glBegin(GL_LINE_LOOP);
			for(int j=0; j<ORBIT_PRECISION; j++) {
				glVertex3d(planet[i].get_orbit_line(j).x-camera.focus.x,planet[i].get_orbit_line(j).y-camera.focus.y,planet[i].get_orbit_line(j).z-camera.focus.z);
			}
			glEnd();
		}
	}

	//Wyúwetl s≥oÒce
	glBindTexture(GL_TEXTURE_2D, planet_textures[0].GetID());
	CreateSphere(zero_moved,planet[0].get_diameter()/2,40);

	//Wyúwietl kolejno wszystkie planety uk≥adu
	for (int i=1; i<10; i++) {
		//Zerujemy macierz przkeszta≥ceÒ
		glLoadIdentity();
		//Orientujemy scenÍ wzglÍdem kamery
		glTranslatef(0,0,-pow((long double)10.0,(long double)camera.radius/50.0)+camera.inf_radius);
		glRotated(camera.alpha,1,0,0);
		glRotated(camera.theta,0,1,0);
		//Dokonujemy korekcji p≥aszczyzny ekliptyki
		glRotated(-90,1,0,0);
		//Przesuwamy siÍ w miejsce i-tej planety
		glTranslatef(planet[i].get_sun_position().x-camera.focus.x,planet[i].get_sun_position().y-camera.focus.y,planet[i].get_sun_position().z-camera.focus.z);
		//Pochylamy sie o kπt odpowiadajπcy odchyleniu osi
		glRotated(planet[i].get_axial_tilt(),1,0,0);
		//Obliczamy kπt o jaki naleøy obruciÊ kamerÍ
		double long d, jd;
		d=planet[i].get_rotation_speed();
		jd=timer.julian_day();
		jd=jd-floor(jd/d)*d;
		//Obracamy planetÍ o wyliczony kπt
		glRotated(-360.0*jd/d,0,0,1);
		//Wyúwietlamy planetÍ
		glBindTexture(GL_TEXTURE_2D, planet_textures[i].GetID());
		CreateSphere(zero,planet[i].get_diameter()/2,40);
		//Cofamy czÍúÊ przekszta≥ceÒ by mÛc poprwnie obliczyÊ ich pozycjÍ na ekranie
		glRotated(360.0*jd/d,0,0,1);
		glRotated(-planet[i].get_axial_tilt(),1,0,0);
		glTranslatef(-planet[i].get_sun_position().x+camera.focus.x,-planet[i].get_sun_position().y+camera.focus.y,-planet[i].get_sun_position().z+camera.focus.z);
		//Obliczamy pozycjÍ planety na ekrania
		planet[i].calculate_screen_position(camera.focus);
	}
	
	//Wyúwetlamy pierúcienie saturna (analogicznie do wyúwietlenia planety
	//Poniewaø wyúwietlenie tego elementu wykoøystuje mieszanie kolorÛw to musi zostaÊ
	//zrobione po wyúwietleniu wszystkich innych, ktÛre ewentualnie mogπ znaleüÊ siÍ za nim.
	glLoadIdentity();
	glTranslatef(0,0,-pow((long double)10.0,(long double)camera.radius/50.0)+camera.inf_radius);
	glRotated(camera.alpha,1,0,0);
	glRotated(camera.theta,0,1,0);
	glRotated(-90,1,0,0);
	glTranslatef(planet[6].get_sun_position().x,planet[6].get_sun_position().y,planet[6].get_sun_position().z);
	glRotated(planet[6].get_axial_tilt(),1,0,0);
	glRotated(90,1,0,0);
	glBindTexture(GL_TEXTURE_2D,saturn_ring.GetID());
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
	glBegin(GL_QUADS);
		glTexCoord2f(0,1);
		glVertex3f(-3,0,-3);
		glTexCoord2f(1,1);
		glVertex3f(3,0,-3);
		glTexCoord2f(1,0);
		glVertex3f(3,0,3);
		glTexCoord2f(0,0);
		glVertex3f(-3,0,3);
	glEnd();

	//Wyúwietl interaktywne etykietki, ktÛre pozwolπ na orionetowanie widoku wzglÍdem danej planety
	glLoadIdentity();
	for (int i=1; i<10; i++) {
		if (planet[i].get_screen_position().z==0) {
			aux_rect.left=planet[i].get_screen_position().x-Interface.LabelsGFX[i].GetWidth()/2;
			aux_rect.top=planet[i].get_screen_position().y-Interface.LabelsGFX[i].GetHeight()/2;
			aux_rect.right=aux_rect.left+Interface.LabelsGFX[i].GetWidth();
			aux_rect.bottom=aux_rect.top+Interface.LabelsGFX[i].GetHeight();
			Interface.Labels[i].InitButton(aux_rect,&Interface.LabelsGFX[i],&Interface.LabelsGFX[i],&Interface.LabelsGFX[i]);
			//Jeúli rysowanie etykietek jest w≥πczone to rysuj je na ekranie
			if (Interface.Buttons[BT_LABELS].IsActive()) Interface.Labels[i].PrintButton(false,aux_rect.left,aux_rect.top);
		}
	}

	//Wyúwietl nazwe programu
	title.PrintBMP((WINDOW_WIDTH-title.GetWidth())/2,5);

	//Wyúwietl belkÍ na, ktÛrej znajda siÍ wszytskie elementy interface'u
	Interface.Panel.PrintBMP(160,746);

	//Jeúli przycisk informacji jest w≥πczone to wyúwietl odpowiednie okienko
	if (Interface.Buttons[BT_INFO].IsActive()) Interface.InfoWindows[selected].PrintBMP(0,0);
	//jeúli nie to wyúwietl okienko zminimalizowane
	else Interface.InfoWindows[10].PrintBMP(0,0);

	//Wyúwietl wszytskie przycikski Interface'u
	for(int i=0; i<12; i++){
		//Jeúli guzik jest naciskany to wyúwietl go jako podúwietlonego
		if (lmb_pressed && Interface.Buttons[i].IsInArea(mouse_x,mouse_y)) {
			Interface.Buttons[i].PrintButton(true,Interface.Buttons[i].rcArea.left,Interface.Buttons[i].rcArea.top);
		//Jeúli nie to wyúwietl go zgodnie z jego stanem
		} else {
			Interface.Buttons[i].PrintButton(false,Interface.Buttons[i].rcArea.left,Interface.Buttons[i].rcArea.top);
		}
	}

	//Jeúli zegar jest w trybie edycji to wyúwietl odpowiedni znacznik
	if (isClockEditing) {
		for (int i=0; i<7; i++) {
			Interface.ClockHighlight[i].PrintButton(false,Interface.ClockHighlight[i].rcArea.left,Interface.ClockHighlight[i].rcArea.top);
		}
	}

	//Wyúwietl elementy zegara
	Interface.ClockBC[timer.bc].PrintBMP(628,753);
	DrawNumber(timer.year,663,754,4);
	Interface.ClockMonths[timer.month-1].PrintBMP(698,753);
	DrawNumber(timer.day,768,754,2);
	DrawNumber(timer.hour,804,754,2);
	DrawNumber(timer.minute,823,754,2);
	DrawNumber(timer.second,842,754,2);

	//ZamieÒ bufory
	SwapBuffers(hDC);
};

//FUNKCJA ODCZYTUJE DANE Z MYSZKI I KLAWIATURY I REAGUJE NA NIE

void ReadInput(){
	//Zmienna pomocnicza przy ustalaniu wspÛlrzÍdnych kursoru
	POINT help;

	//Ustawia flagÍ, øe nie zosta≥ wciúniÍty øaden przycisk na true
	no_click=true;

	//Odczytywanie pozycji kursora
	GetCursorPos(&help);
	mouse_x = help.x;
	mouse_y = help.y;

	//Ustawienie odpowiednich flag okreúlajπcych pozycjÍ lewego kalwisza myszki (czy jest wciúniÍty lub czy zosta≥ zwolniony)
	if (HIBYTE(GetAsyncKeyState(VK_LBUTTON))!=0) {
		lmb_pressed=true;
	} else {
		if (lmb_pressed && !lmb_released) {
			lmb_pressed=false;
			lmb_released=true;
		} else {
			lmb_released=false;
		}
	}

	//Ustawia flagÍ okreúlajπcπ pozycjÍ prawego kursora myszki
	if (HIBYTE(GetAsyncKeyState(VK_RBUTTON))!=0) {
		rmb_pressed=true;
	} else {
		rmb_pressed=false;
	}

	//Poniewaø erÍ zmienia siÍ pojedynczym klikniÍciem, przy kaødym przebiegu pÍtli programu wy≥πcz jej przycisk
	Interface.ClockHighlight[CH_AGE].Disable();

	//Jeúli lewy klawisz myszy jest wciúniÍty
	if (lmb_pressed) {
		//Jeúli znajduje siÍ na suwaku to aktywuj go i ustaw zmiennπ delta (s≥uøy ona po to by przy pojedynczym klikniÍciu na suwak nie przesuwa≥ sie on.)
		if (Interface.Buttons[BT_SLIDE].IsInArea(mouse_x,mouse_y) && !Interface.Buttons[BT_SLIDE].IsActive()) {
			Interface.Buttons[BT_SLIDE].Enable();
			delta = mouse_x-Interface.Buttons[BT_SLIDE].rcArea.left;
		}
		//Jeúli suwak jest naciúniÍty to
		if (Interface.Buttons[BT_SLIDE].IsActive()) {
			//Jeúli jest w ruchu to co trzeci pixel wyúwietl düwiÍk
			if (Interface.Buttons[BT_SOUND].IsActive() && mouse_x-delta+10-450!=time_speed && time_speed%3==0 && time_speed!=135 && time_speed!=-135) {
				DSound_Play(sound_slide);
				DSound_Set_Freq(sound_slide,44100);
			}
			//Ustaw prÍdkoúÊ up≥ywu czasu zgodnie z pozycjπ myszy
			time_speed = mouse_x-delta+10-450;

			//Sprawdü czy prÍdkoúÊ czasu znajuje siÍ w ustalonych granicach, a jeúli nie to skoryguj tÍ wartoúÊ
			if (time_speed<-135) time_speed=-135;
			if (time_speed>135) time_speed=135;
			
			//Ustaw suwak zgodnie z prÍdkoúciπ up≥ywu czasu 
			Interface.Buttons[BT_SLIDE].rcArea.left = 450+time_speed-10;
			Interface.Buttons[BT_SLIDE].rcArea.right = Interface.Buttons[BT_SLIDE].rcArea.left + 24;
		}
		//Skoro zosta≥ wciúniÍty lewy prycisk myszy to zaprzestaÒ edycji zegara
		isClockEditing=false;
		for(int i=0; i<7; i++){
			Interface.ClockHighlight[i].Disable();
		}
	} else {
		//Jeúli lewy przycisk zwolniony...
		if (lmb_released) {
			//Przyciski reagujπ tak na prawdÍ gdy zwolni siÍ przycisk myszy gdy kursor znajduje siÍ nad ktÛrymú z nich
			for (int i=0; i<12; i++){
				//Jeúli kursor znajdowa≥ siÍ na przycisku exit to go w≥πcz (w konsekwencji spowoduje to przerwanie g≥Ûwnej pÍtli programu 
				if (i==BT_EXIT && Interface.Buttons[BT_EXIT].IsInArea(mouse_x,mouse_y)) {
					//Ustaw flagÍ, øe nie zosta≥ wciúniÍty øaden przycisk na false
					no_click=false;
					Interface.Buttons[BT_EXIT].Enable();
					//OdtwÛrz düwiÍk wciskanego przycisku
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_button);
						DSound_Set_Freq(sound_button,44100);
					}
				//Jeúli kursor znajdowal siÍ nad przyciskiem widoku z gÛry
				} else if (i==BT_BIRDVIEW && Interface.Buttons[BT_BIRDVIEW].IsInArea(mouse_x,mouse_y)) {
					no_click=false;
					Interface.Buttons[BT_BIRDVIEW].Enable();
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_button);
						DSound_Set_Freq(sound_button,44100);
					}
					//Ustaw kamerÍ tak by patrzy≥a prostopadle na ekliptykÍ
					camera.alpha=90;
				//Jeúli kursor by≥ na suwaku
				} else if (i==BT_SLIDE && Interface.Buttons[BT_SLIDE].IsActive()) {
					no_click=false;
					//Wy≥πcz suwak
					Interface.Buttons[BT_SLIDE].Disable();
				//Jeúli kursor znajdowa≥ siÍ na przycisku ustawiajπcym up≥yw czasu na
				} else if (i==BT_RESET_SPEED && Interface.Buttons[BT_RESET_SPEED].IsInArea(mouse_x,mouse_y)) {
					no_click=false;
					Interface.Buttons[BT_RESET_SPEED].Enable();
					//Ustaw prÍdkoúÊ up≥ywu czasu (prÍdkoúÊ rzeczywista)
					time_speed=0;
					//PrzesuÒ suwak zgodnie z aktualnπ prÍdkoúciπ
					Interface.Buttons[BT_SLIDE].rcArea.left = 450+time_speed-10;
					Interface.Buttons[BT_SLIDE].rcArea.right = Interface.Buttons[BT_SLIDE].rcArea.left + 24;
					//OdtwÛrz düwiÍk wciskanego przycisku
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_button);
						DSound_Set_Freq(sound_button,44100);
					}
				//Jeúli kursor znajdowa≥ siÍ na przycisku resetujπcym wartoúÊ zegara
				} else if (i==BT_RESET_TIME &&  Interface.Buttons[BT_RESET_TIME].IsInArea(mouse_x,mouse_y)) {
					no_click=false;
					//Ustaw zegar na aktualny czas
					timer.set_current_time();
					//OdtwÛrz düwiÍk wciskanego przycisku
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_button);
						DSound_Set_Freq(sound_button,44100);
					}
				//Jeúli zosta≥ wciúniÍty przycisk w≥πczjπcy muzykÍ
				} else if(i==BT_MUSIC &&  Interface.Buttons[BT_MUSIC].IsInArea(mouse_x,mouse_y)) {
					no_click=false;
					//Jeúli przycisk by≥ aktywny to go wy≥acz i wy≥acz muzykÍ
					if (Interface.Buttons[i].IsActive()) {
						Interface.Buttons[i].Disable();
						DSound_Stop_Sound(music);
					//Jeúli nie to go w≥πcz i w≥πcz muzykÍ
					} else {
						Interface.Buttons[i].Enable();
						DSound_Play(music,1);
						DSound_Set_Volume(music,5);
						DSound_Set_Freq(music,44100);
					}
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_button);
						DSound_Set_Freq(sound_button,44100);
					}
				//Pozosta≥e przyciski obs≥uø standardowo
				} else if (Interface.Buttons[i].IsInArea(mouse_x,mouse_y)) {
					no_click=false;
					//ZmieÒ ustawienie przycisku
					if (Interface.Buttons[i].IsActive()) Interface.Buttons[i].Disable();
					else Interface.Buttons[i].Enable();
					//OdtwÛrz düwiÍk
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_button);
						DSound_Set_Freq(sound_button,44100);
					}
				}
			}
			//Sprawdü czy kursor znajdowa≥ siÍ nad ktÛrymú elementem zegara
			for(int i=0; i<7; i++){
				if (Interface.ClockHighlight[i].IsInArea(mouse_x,mouse_y)) {
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_button);
						DSound_Set_Freq(sound_button,44100);
					}
					no_click=false;
					//Aktywuj konkretny element zegara
					Interface.ClockHighlight[i].Enable();
					if (i==CH_AGE) {
						//Jeúli by≥a to era to nie wchodü w tryb edycji tylko zmieÒ jej wartoúÊ
						timer.bc=!timer.bc;
					} else {
						//W pozosta≥ych przypadkach przejdü w tryb edycji zegara
						isClockEditing=true;
						//Ponadto jeúli by≥ to rok to ustaw aktualnie edytowanπ cyfrÍ na 3
						if (i==CH_YEAR) {
							year_digit_editing=3;
						}
					}
				}
			}
			//Sprawdü czy zosta≥a klikniÍta ktÛraú z interaktywnych etykietek planet
			for (int i=1; i<10; i++) {
				if (Interface.Labels[i].IsInArea(mouse_x,mouse_y) && planet[i].get_screen_position().z==0) {
					no_click=false;
					//Jeúli tak to ustaw aktualnie zaznaczonπ planetÍ
					selected=i;
					//Inicjuj proces zmiany i interpolacji punktu na, ktÛry patrzy kamera i minimalnego promienia
					camera.focus_interpolation=30;
					//OddtwÛrz odpowiedniπ etykietkÍ düwiÍkowπ
					if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_labels[i-1]);
						DSound_Set_Volume(sound_labels[i-1],60);
						DSound_Set_Freq(sound_labels[i-1],44100);
					}
					break;
				}
			}
			//Jeúli klikniÍtπ w przestrzeÒ, czyli nie klikniÍto na øaden z elementÛw interface'u
			if (no_click) {
				//Ustaw aktualnie zaznaczony obiekt na 0, czyli s≥oÒce
				selected=0;
				//Inicjuj zmianÍ centralnego punktu
				camera.focus_interpolation=30;
			}
		}
	}
	//Jeúli zosta≥ wykonany ruch rolkπ myszy
	if (mouse_roll!=0) {
		//Jeúli program nie jest w trybie edycji zegara
		if (!isClockEditing) {
			//ZmieÒ odleg≥oúÊ widzenia o wartoúÊ odchylenia rolki (1 lub -1)
			camera.radius-=mouse_roll;
			//Jeúli promieÒ przekracza wartoúci graniczne to skoryguj go
			if (camera.radius<0) camera.radius=0;
			if (camera.radius>100)camera.radius=100;
		//Jesli program jest w trybie edycji zegar to zmodyfikuj odpowiedni jego element
		} else for (int i=1; i<7; i++) {
			if (Interface.ClockHighlight[i].IsActive()) {
				switch(i) {
					case CH_YEAR: {
						timer.year+=mouse_roll;
					}break;
					case CH_MONTH: {
						timer.month+=mouse_roll;
						if (timer.month<1) {
							timer.month=1;
							//OdtwÛrz düwiÍk b≥edu edycji zegara
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
						} 
						if (timer.month>12) {
							timer.month=12;
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
						} 
					}break;
					case CH_DAY: {
						timer.day+=mouse_roll;
						if (timer.day<1) timer.day=1;
						//GÛrna wartoúÊ dnia zaleøy od aktualnego miesiπca i roku (lata przestÍpne) co sprawdza ciπg poniøszych warunkÛw
						//Jeúi zostanie wykryty jakiú b≥πd to w kaødym przypadku wyúwietl düwiÍk b≥edu edycji zegara
						if ((timer.month<8 && timer.month%2==1) || (timer.month>7 && timer.month%2==0)) {
							if (timer.day>31) {
								timer.day=31;
								if (Interface.Buttons[BT_SOUND].IsActive()) {
									DSound_Play(sound_timer_edit_error);
									DSound_Set_Freq(sound_timer_edit_error,44100);
								}
							}
						} else if (timer.month==2) {
							if (timer.year%4==0 && timer.year>=1582) {
								if (timer.day>29) {
									timer.day=29;
									if (Interface.Buttons[BT_SOUND].IsActive()) {
										DSound_Play(sound_timer_edit_error);
										DSound_Set_Freq(sound_timer_edit_error,44100);
									}
								}
							} else {
								if (timer.day>28) {
									timer.day=28;
									if (Interface.Buttons[BT_SOUND].IsActive()) {
										DSound_Play(sound_timer_edit_error);
										DSound_Set_Freq(sound_timer_edit_error,44100);
									}
								}
							}
						} else {
							if (timer.day>30) {
								timer.day=30;
								if (Interface.Buttons[BT_SOUND].IsActive()) {
									DSound_Play(sound_timer_edit_error);
									DSound_Set_Freq(sound_timer_edit_error,44100);
								}
							}
						}
					}break;
					case CH_HOUR: {
						timer.hour+=mouse_roll;
						if (timer.hour<0) {
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
							timer.hour=0;
						}
						if (timer.hour>23) {
							timer.hour=23;
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
						}
					}break;
					case CH_MINUTE: {
						timer.minute+=mouse_roll;
						if (timer.minute<0) {
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
							timer.minute=0;
						}
						if (timer.minute>59) {
							timer.minute=59;
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
						}
					}break;
					case CH_SECONDS: {
						timer.second+=mouse_roll;
						if (timer.second<0) {
							timer.second=0;
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
						}
						if (timer.second>59) {
							timer.second=59;
							if (Interface.Buttons[BT_SOUND].IsActive()) {
								DSound_Play(sound_timer_edit_error);
								DSound_Set_Freq(sound_timer_edit_error,44100);
							}
						}
					}break;
				}
				//OdtwÛrz düwiÍk modyfikacji wartoúci zegara
				if (Interface.Buttons[BT_SOUND].IsActive()) {
					DSound_Play(sound_timer_edit);
					DSound_Set_Freq(sound_timer_edit,44100);
				}
			}
		}
	}
	//Ustaw wartoúÊ odchylenia rolki z powrotem na zero gdyø w pÍtli zdarzeniowej okna
	//nie zostanie jej przypisana øadna wartoúÊ jeúli nie zostanie wykonany ruch rolkπ
	mouse_roll=0;
	//Jeúli jest wcisniÍty prawy klawisz myszy
	if (rmb_pressed) {
		//Zmodyfikuj kπty widzenia kamery zgodnie z przesuniÍciem myszki 
		camera.theta+=(mouse_x-old_mouse_x)/4.0;
		camera.alpha+=(mouse_y-old_mouse_y)/4.0;
		if (camera.theta<0) camera.theta=360+camera.theta;
		if (camera.theta>360) camera.theta=camera.theta-360;
		if (camera.alpha>90) camera.alpha=90;
		if (camera.alpha<-90) camera.alpha=-90;
	}
	//Ustaw poprzednie wspÛ≥rzedne kursora po to by w nastÍpnym przebiegu pÍtli programu bu≥a znana jego pozycja w poprzedniej klatce 
	old_mouse_x=mouse_x;
	old_mouse_y=mouse_y;
	//Jeúli jest edytowany rok
	if (Interface.ClockHighlight[CH_YEAR].IsActive()){
		//Zmniejsz opuünienie klawiatury
		keyboard_delay--;
		//Jeúli jest mniejsze od zera to ustaw je na zero
		if (keyboard_delay<0) keyboard_delay=0;
		//Jeúli opuünienie osiπgnÍ≥o zero to...
		if (keyboard_delay==0) {
			//Odczytaj czy nie zosta≥ wciúniÍty jakiú klawisz numeryczny
			for (int i=0; i<10; i++) {
				if (keys[VK_NUMPAD0+i]==true) {
					//Jeúli tak to sprawdü czy uøytkownik nie prÛbuje na pierwszym miejscu wstawiÊ trÛjki, albo liczby wiÍkszej od 3 (max. 3000 n.e., min. 3000 p.n.e.)
					if (year_digit_editing!=3 || i<3) {
						//Jeúli wszytsko jest w porzπdku to zamieÒ atualnie edytowanπ cyfrÍ na tπ wybranπ przez uøytkownika
						int help = pow(10,year_digit_editing);
						timer.year+=(i-(timer.year/help)%10)*help;
						//Zmniejsz aktualnie edytowanπ cyfrÍ o 1
						year_digit_editing--;
						//Jeúli wsyztskie cyfry roku zosta≥y wpisane to wrÛÊ do pierwszej
						if (year_digit_editing<0) year_digit_editing=3;
						//Ustaw opuünienie klawiatury
						keyboard_delay=10;
						if (Interface.Buttons[BT_SOUND].IsActive()) {
							DSound_Play(sound_timer_edit);
							DSound_Set_Freq(sound_timer_edit,44100);
						}
					} else if (Interface.Buttons[BT_SOUND].IsActive()) {
						DSound_Play(sound_timer_edit_error);
						DSound_Set_Freq(sound_timer_edit_error,44100);
					}
				}
			}
		}
	}
	//Jeúli rok jest mniejszy od jeden to zmieÒ erÍ
	if (timer.year<1) {
		timer.bc=!timer.bc;
		timer.year=1;
	}
	//Jeúli data przekracza datÍ granicznπ to ustaw jπ na najwyøszπ/najniøszπ z moøliwych
	if (timer.year>3000) {
		timer.year=3000;
		if (timer.bc) {
			timer.month=1;
			timer.day=1;
			timer.hour=0;
			timer.minute=0;
			timer.second=0;
		} else {
			timer.month=12;
			timer.day=31;
			timer.hour=23;
			timer.minute=59;
			timer.second=59;
		}
	}
	//Jeúli kamera nie jest prostopad≥a do ekliptyki to wy≥πcz guzik widoku z gÛry
	if (camera.alpha!=90) Interface.Buttons[BT_BIRDVIEW].Disable();
	//Jeúli prÍdkoúÊ up≥ywu czasu jest rÛwn zero to w≥πcz przycisk zerowania prÍdkoúci
	if (time_speed==0) Interface.Buttons[BT_RESET_SPEED].Enable();
	//Jeúli nie to go wy≥πcz
	else Interface.Buttons[BT_RESET_SPEED].Disable();
};

// G£”WNA FUNKCJA PROGRAMU

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG		msg;			// Zmeinna przechowujπca komunikaty systemowe
	msg.message=0;			// Ustaw pierwszy komuniakt na rÛøny od WM_QUIT po to by pÍtla programu nie zosta≥a przerwana od razu
	clock_t begin_time=0;   // Czas na pcozπdku 
	clock_t end_time=0;
	Init();
	while(msg.message!=WM_QUIT && !Interface.Buttons[BT_EXIT].IsActive()){
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
		} else {
			if (active) {
				//Opuünij dzia≥anie programu o 1/60 sekundy, chyba, øe wyúwietlanie poprzedniej kratki trwa≥o d≥uøej niø 1/1000, wÛwczas modyfikuj opuünienie
				if (end_time-begin_time<15) Sleep(15 - end_time + begin_time);
				//ZwiÍksz wartoúÊ zegara zgodnie z prÍdkoúÊia up≥ywu czasu
				//Podobnie jak przy oddalenie widoku , rzeczywista prÍdkoúÊ up≥ywu czasu jest 
				//tutaj funcjπ wyk≥adnicznπ wprowadzonπ po to by przy ma≥ych odchyleniach suwaka i ma≥ych prÍdkoúciach uøytkownik mÛg≥
				//z wiÍkszπ precyzjπ ustawiÊ øπdanπ prÍdkoúÊ niø przy bardzo duøych odchyleniach, gdzie planety
				//poruszajπ siÍ tak szybko øe nie juø w sumie znaczenia jaka to prÍdkoúÊ
				if (!Interface.Buttons[BT_PAUSE].IsActive()) timer.inc_time((15 - end_time + begin_time)*(time_speed == 0 ? 1:(time_speed/abs(time_speed))*pow(10.0,7.0/150.0*abs(time_speed))*48));
				//Zapisz czas na poczπdku wykonania pÍtli
				begin_time = clock();
				ReadInput();
				//Oblicz pozycje wszytskich planet
				for (int i=1; i<10; i++) planet[i].calculate_sun_positions(timer.julian_day());
				//Jesli jest interpolowana pozycja kamery
				if (camera.focus_interpolation>0) {
					//ZapamiÍtaj poprzednie wspÛ≥rzedne punktu centralnego
					camera.old_focus=camera.focus;
					//ZapamiÍtaj poprzedni minimalny promieÒ
					camera.old_inf_radius = camera.inf_radius;
					//Jeúli aktualnym obiektem jest s≥oÒce to dalej interpoluj pozycje kamery w kierunku úrodka uk≥adu s≥onecznego
					if (selected>0) camera.change_focus(planet[selected].get_sun_position().x,planet[selected].get_sun_position().y,planet[selected].get_sun_position().z,0.9-planet[selected].get_diameter());
					//Jeúli nie to dalej interpoluj pozycje kamery ku aktualnei zaznaczonej planecie
					else camera.change_focus(0,0,0,0.9-planet[selected].get_diameter());
				//Jeúli przesuwanie widoku zosta≥o zakoÒczone to ustaw centralny punkt na pozycje danego obiektu
				} else {
					if (selected>0 ) camera.set_focus(planet[selected].get_sun_position().x,planet[selected].get_sun_position().y,planet[selected].get_sun_position().z,0.9-planet[selected].get_diameter());
					else camera.set_focus(0,0,0,0.9-planet[selected].get_diameter());
				}

				DrawScene();
				//Zapisz czas na koÒcu wykonania pÍtli
				end_time = clock();
			}
		}
	}
	Release();
	return msg.wParam;
};
