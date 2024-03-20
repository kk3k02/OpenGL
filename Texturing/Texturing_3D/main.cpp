#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int model = 1;
// Wybor rysowanego obiektu: 1 - czajnik (wire), 2 - czajnik (solid)
// 3 - jajko (siatka), 4 - jajko (trojkaty)

bool wall1 = true;
bool wall2 = true;
bool wall3 = true;
bool wall4 = true;
bool wall5 = true;
// Sterowaniem rysowanej sciany

int perspective = 1;
// Perspektywa obserwatora
// 1 - obrot obiektu
// 2 - zmiana polozenia obserwatora

int mode = 1;
// Wybor trybu dzialania programu
// 1 - obracanie obiektem / ruch obserwatora
// 2 - sterowanie oswietleniem

const double pi = 3.14159265358979323846;

int N = 30; // Liczba przedzialow boku kwadratu

typedef float point3[3];

static GLfloat viewer[] = { 0.0, 0.0, 10.0 };
// inicjalizacja po�o�enia obserwatora

static GLfloat theta = 0.0;
// k�t obrotu obiektu wok� osi x

static GLfloat gamma = 0.0;
// k�t obrotu obiektu wok� osi y

static GLfloat pix2angle;
static GLfloat piy2angle;
// przelicznik pikseli na stopnie

static GLint status_left = 0;
static GLint status_right = 0;
// stan klawiszy myszy
// 0 - nie naci�ni�to �adnego klawisza
// 1 - naci�ni�ty zosta� lewy klawisz

static int delta_x = 0;
// r�nica pomi�dzy pozycj� bie��c�
// i poprzedni� kursora myszy (x)

static int delta_y = 0;
// r�nica pomi�dzy pozycj� bie��c�
// i poprzedni� kursora myszy (y)

static int delta_zoom = 0;
// r�nica pomi�dzy pozycj� bie��c�
// i poprzedni� zoom

static int x_pos_old = 0;
// poprzednia pozycja kursora myszy (x)

static int y_pos_old = 0;
// poprzednia pozycja kursora myszy (y)

static int zoom_pos_old = 0;
// poprzednia pozycja zoom

/*************************************************************************************/
 // Funkcja wczytuje dane obrazu zapisanego w formacie TGA w pliku o nazwie
 // FileName, alokuje pami�� i zwraca wska�nik (pBits) do bufora w kt�rym
 // umieszczone s� dane.
 // Ponadto udost�pnia szeroko�� (ImWidth), wysoko�� (ImHeight) obrazu
 // tekstury oraz dane opisuj�ce format obrazu wed�ug specyfikacji OpenGL
 // (ImComponents) i (ImFormat).
 // Jest to bardzo uproszczona wersja funkcji wczytuj�cej dane z pliku TGA.
 // Dzia�a tylko dla obraz�w wykorzystuj�cych 8, 24, or 32 bitowy kolor.
 // Nie obs�uguje plik�w w formacie TGA kodowanych z kompresj� RLE.
/*************************************************************************************/


GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat)
{

    /*************************************************************************************/

    // Struktura dla nag��wka pliku  TGA


#pragma pack(1)           
    typedef struct
    {
        GLbyte    idlength;
        GLbyte    colormaptype;
        GLbyte    datatypecode;
        unsigned short    colormapstart;
        unsigned short    colormaplength;
        unsigned char     colormapdepth;
        unsigned short    x_orgin;
        unsigned short    y_orgin;
        unsigned short    width;
        unsigned short    height;
        GLbyte    bitsperpixel;
        GLbyte    descriptor;
    }TGAHEADER;
#pragma pack(8)

    FILE* pFile;
    TGAHEADER tgaHeader;
    unsigned long lImageSize;
    short sDepth;
    GLbyte* pbitsperpixel = NULL;


    /*************************************************************************************/

    // Warto�ci domy�lne zwracane w przypadku b��du

    *ImWidth = 0;
    *ImHeight = 0;
    *ImFormat = GL_BGR_EXT;
    *ImComponents = GL_RGB8;

    if (fopen_s(&pFile, FileName, "rb") != 0 || pFile == NULL) {
        // B��d otwarcia pliku
        return NULL;
    }

    /*************************************************************************************/
    // Przeczytanie nag��wka pliku 


    fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);


    /*************************************************************************************/

    // Odczytanie szeroko�ci, wysoko�ci i g��bi obrazu

    *ImWidth = tgaHeader.width;
    *ImHeight = tgaHeader.height;
    sDepth = tgaHeader.bitsperpixel / 8;


    /*************************************************************************************/
    // Sprawdzenie, czy g��bia spe�nia za�o�one warunki (8, 24, lub 32 bity)

    if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)
        return NULL;

    /*************************************************************************************/

    // Obliczenie rozmiaru bufora w pami�ci


    lImageSize = tgaHeader.width * tgaHeader.height * sDepth;


    /*************************************************************************************/

    // Alokacja pami�ci dla danych obrazu


    pbitsperpixel = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));

    if (pbitsperpixel == NULL)
        return NULL;

    if (fread(pbitsperpixel, lImageSize, 1, pFile) != 1)
    {
        free(pbitsperpixel);
        return NULL;
    }


    /*************************************************************************************/

    // Ustawienie formatu OpenGL


    switch (sDepth)

    {

    case 3:

        *ImFormat = GL_BGR_EXT;

        *ImComponents = GL_RGB8;

        break;

    case 4:

        *ImFormat = GL_BGRA_EXT;

        *ImComponents = GL_RGBA8;

        break;

    case 1:

        *ImFormat = GL_LUMINANCE;

        *ImComponents = GL_LUMINANCE8;

        break;

    };



    fclose(pFile);



    return pbitsperpixel;

}

/*************************************************************************************/

// Funkcja obliczajaca wspolrzedne powierzchni opisanej rownaniami parametrycznymi

float calculate(string xyz, float u, float v)
{

    if (u >= 0 && u <= 1 && v >= 0 && v <= 1)
    {
        float result = 0.0;

        // x(u, v)
        if (xyz == "x")
        {
            result = (-90.0 * powf(u, 5) + 225.0 * powf(u, 4) - 270.0 * powf(u, 3) + 180.0 * powf(u, 2) - 45.0 * u) * cos(float(pi) * v);
        }
        // y(u, v)
        else if (xyz == "y")
        {
            result = (160.0 * powf(u, 4) - 320.0 * powf(u, 3) + 160 * powf(u, 2));
        }
        // z(u, v)
        else if (xyz == "z")
        {
            result = (-90.0 * powf(u, 5) + 225.0 * powf(u, 4) - 270.0 * powf(u, 3) + 180.0 * powf(u, 2) - 45.0 * u) * sin(float(pi) * v);
        }
        else if (xyz == "ux")
        {
            result = (-450.0 * powf(u, 4) + 900.0 * powf(u, 3) - 810.0 * powf(u, 2) + 360.0 * u - 45.0) * cos(float(pi) * v);
        }
        else if (xyz == "vx")
        {
            result = float(pi) * (90.0 * powf(u, 5) - 225.0 * powf(u, 4) + 270.0 * powf(u, 3) - 180.0 * powf(u, 2) + 45.0) * sin(float(pi) * v);
        }
        else if (xyz == "uy")
        {
            result = (640.0 * powf(u, 3) - 960.0 * powf(u, 2) + 320.0 * u);
        }
        else if (xyz == "vy")
        {
            result = 0.0;
        }
        else if (xyz == "uz")
        {
            result = (-450.0 * powf(u, 4) + 900.0 * powf(u, 3) - 810.0 * powf(u, 2) + 360.0 * u - 45.0) * sin(float(pi) * v);
        }
        else if (xyz == "vz")
        {
            result = -float(pi) * (90.0 * powf(u, 5) - 225.0 * powf(u, 4) + 270.0 * powf(u, 3) - 180.0 * powf(u, 2) + 45.0) * cos(float(pi) * v);
        }

        return result;
    }

    return NULL;
}

/*************************************************************************************/

// Funkcja obliczajaca wartosci wektora normalnego
float calculate_vector(char xyz, float u, float v)
{
    float normal = 0.0;

    if (xyz == 'x' || xyz == 'y' || xyz == 'z')
    {
        if (xyz == 'x')
        {
            normal = (calculate("uy", u, v) * calculate("vz", u, v)) - (calculate("uz", u, v) * calculate("vy", u, v));
        }
        if (xyz == 'y')
        {
            normal = (calculate("uz", u, v) * calculate("vx", u, v)) - (calculate("ux", u, v) * calculate("vz", u, v));
        }
        if (xyz == 'z')
        {
            normal = (calculate("ux", u, v) * calculate("vy", u, v)) - (calculate("uy", u, v) * calculate("vx", u, v));
        }

        return normal;
    }

    return NULL;
}

/*************************************************************************************/

// Funkcja normalizujaca dlugosci wektorow normalnych

float normalize_vector(float x, float y, float z)
{
    return sqrt(powf(x, 2) + powf(y, 2) + powf(z, 2));
}

/*************************************************************************************/

// Funkcja rysujaca obiekt w postaci wypelnionych trojkatow

void egg_2(void)
{
    // Inicjalizacja generatora liczb pseudolosowych
    srand(static_cast<unsigned int>(time(nullptr)));

    //glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLES);

    // Rysowanie trojkatw jajka
    for (int i = 0; i < N - 1; i++)
    {
        
        for (int j = 0; j < N - 1; j++)
        {
            float u0 = (1.0 / (N - 1)) * i;
            float v0 = (1.0 / (N - 1)) * j;

            float u1 = (1.0 / (N - 1)) * (i + 1);
            float v1 = (1.0 / (N - 1)) * j;

            float u2 = (1.0 / (N - 1)) * i;
            float v2 = (1.0 / (N - 1)) * (j + 1);

            float u3 = (1.0 / (N - 1)) * (i + 1);
            float v3 = (1.0 / (N - 1)) * (j + 1);

            float x, y, z, normalized;
            

            // Pierwszy trojkat
            x = calculate_vector('x', u0, v0);
            y = calculate_vector('y', u0, v0);
            z = calculate_vector('z', u0, v0);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;

            glNormal3f(x, y, z);
            glTexCoord2f(v0, u0);  
            glVertex3f(calculate("x", u0, v0), calculate("y", u0, v0), calculate("z", u0, v0));

            x = calculate_vector('x', u1, v1);
            y = calculate_vector('y', u1, v1);
            z = calculate_vector('z', u1, v1);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glTexCoord2f(v1, u1);  
            glVertex3f(calculate("x", u1, v1), calculate("y", u1, v1), calculate("z", u1, v1));

            x = calculate_vector('x', u2, v2);
            y = calculate_vector('y', u2, v2);
            z = calculate_vector('z', u2, v2);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glTexCoord2f(v2, u2);  
            glVertex3f(calculate("x", u2, v2), calculate("y", u2, v2), calculate("z", u2, v2));

            // Drugi trojkat
            x = calculate_vector('x', u1, v1);
            y = calculate_vector('y', u1, v1);
            z = calculate_vector('z', u1, v1);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glTexCoord2f(v1, u1); 
            glVertex3f(calculate("x", u1, v1), calculate("y", u1, v1), calculate("z", u1, v1));

            x = calculate_vector('x', u3, v3);
            y = calculate_vector('y', u3, v3);
            z = calculate_vector('z', u3, v3);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glTexCoord2f(v3, u3);  
            glVertex3f(calculate("x", u3, v3), calculate("y", u3, v3), calculate("z", u3, v3));

            x = calculate_vector('x', u2, v2);
            y = calculate_vector('y', u2, v2);
            z = calculate_vector('z', u2, v2);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);  
            glTexCoord2f(v2, u2);
            glVertex3f(calculate("x", u2, v2), calculate("y", u2, v2), calculate("z", u2, v2));
        }
    }

    glEnd();
}

/*************************************************************************************/

// Funkcja rysuj�ca osie uk�adu wsp�?rz?dnych
void Axes(void)
{

    point3  x_min = { -5.0, 0.0, 0.0 };
    point3  x_max = { 5.0, 0.0, 0.0 };
    // pocz?tek i koniec obrazu osi x

    point3  y_min = { 0.0, -5.0, 0.0 };
    point3  y_max = { 0.0,  5.0, 0.0 };
    // pocz?tek i koniec obrazu osi y

    point3  z_min = { 0.0, 0.0, -5.0 };
    point3  z_max = { 0.0, 0.0,  5.0 };
    //  pocz?tek i koniec obrazu osi y
    glColor3f(1.0f, 0.0f, 0.0f);  // kolor rysowania osi - czerwony
    glBegin(GL_LINES); // rysowanie osi x
    glVertex3fv(x_min);
    glVertex3fv(x_max);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);  // kolor rysowania - zielony
    glBegin(GL_LINES);  // rysowanie osi y

    glVertex3fv(y_min);
    glVertex3fv(y_max);
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);  // kolor rysowania - niebieski
    glBegin(GL_LINES); // rysowanie osi z

    glVertex3fv(z_min);
    glVertex3fv(z_max);
    glEnd();

}

/*************************************************************************************/
// Funkcja rysuje trojkat
void Triangle()
{
    glBegin(GL_TRIANGLES);

    // Pierwszy wierzcho�ek
    glNormal3f(0.0f, 0.0f, 1.0f);     
    glTexCoord2f(0.5f, 1.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);   

    // Drugi wierzcho�ek
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-5.0f, -5.0f, 0.0f);

    // Trzeci wierzcho�ek
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(5.0f, -5.0f, 0.0f);

    glEnd();
    glFlush();
}

/*************************************************************************************/
// Funkcja rysuje priamide
void Pyramid()
{
    if (wall1)
    {
        // Podstawa piramidy (kwadrat)
        glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-3.0f, 0.0f, -3.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(3.0f, 0.0f, -3.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(3.0f, 0.0f, 3.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-3.0f, 0.0f, 3.0f);
        glEnd();
    }

    // Boczne �ciany piramidy
    glBegin(GL_TRIANGLES);

    if (wall2)
    {
        // Pierwsza �ciana
        glNormal3f(0.0f, 0.4472f, 0.8944f);
        glTexCoord2f(0.5f, 1.0f);
        glVertex3f(0.0f, 6.0f, 0.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(3.0f, 0.0f, -3.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-3.0f, 0.0f, -3.0f);
    }

    if (wall3)
    {
        // Druga �ciana
        glNormal3f(-0.8944f, 0.4472f, 0.0f);
        glTexCoord2f(0.5f, 1.0f);
        glVertex3f(0.0f, 6.0f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(3.0f, 0.0f, 3.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(3.0f, 0.0f, -3.0f);
    }

    if (wall4)
    {
        // Trzecia �ciana
        glNormal3f(0.0f, 0.4472f, -0.8944f);
        glTexCoord2f(0.5f, 1.0f);
        glVertex3f(0.0f, 6.0f, 0.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-3.0f, 0.0f, 3.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(3.0f, 0.0f, 3.0f);
    }

    if (wall5)
    {
        // Czwarta �ciana
        glNormal3f(0.8944f, 0.4472f, 0.0f);  // Odwr�cone normalne
        glTexCoord2f(0.5f, 1.0f);
        glVertex3f(0.0f, 6.0f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-3.0f, 0.0f, -3.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-3.0f, 0.0f, 3.0f);
    }

    glEnd();

    glFlush();
}

/*************************************************************************************/
// Funkcja "bada" stan myszy i ustawia warto�ci odpowiednich zmiennych globalnych

void Mouse(int btn, int state, int x, int y)
{
    if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN && btn != GLUT_RIGHT_BUTTON)
    {
        x_pos_old = x;
        y_pos_old = y;
        status_left = 1;
    }
    else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && btn != GLUT_LEFT_BUTTON)
    {
        zoom_pos_old = y;
        status_right = 1;
    }
    else
    {
        status_left = 0;
        status_right = 0;
    }
}

/*************************************************************************************/
// Funkcja "monitoruje" po�o�enie kursora myszy i ustawia warto�ci odpowiednich
// zmiennych globalnych

void Motion(GLsizei x, GLsizei y)
{
    delta_x = x - x_pos_old;
    delta_y = y - y_pos_old;
    delta_zoom = y - zoom_pos_old;

    x_pos_old = x;
    y_pos_old = y;
    zoom_pos_old = y;

    glutPostRedisplay();
}



/*************************************************************************************/

// Funkcja okre�laj�ca co ma by� rysowane (zawsze wywo�ywana, gdy trzeba
// przerysowa� scen�)
void RenderScene(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Czyszczenie okna aktualnym kolorem czyszcz�cym

    glLoadIdentity();
    // Czyszczenie macierzy bie??cej

    gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // Zdefiniowanie po�o�enia obserwatora
    Axes();
    // Narysowanie osi przy pomocy funkcji zdefiniowanej powy�ej

    if (perspective == 1) {
        if (status_left == 1 && status_right == 0)
        {
            theta += delta_x * pix2angle;
            gamma += delta_y * piy2angle;
        }
        if (status_right == 1 && status_left == 0)
        {
            viewer[2] += delta_zoom * pix2angle * 0.1; // Modyfikacja wsp�czynnika zooma (0.1 to arbitralna warto��)
        }
    }
    if (perspective == 2)
    {
        if (status_left == 1 && status_right == 0)
        {
            viewer[0] += delta_x * pix2angle * 0.1;
            viewer[1] += delta_y * piy2angle * 0.1;
        }
        if (status_right == 1 && status_left == 0)
        {
            viewer[2] += delta_zoom * pix2angle * 0.1; // Modyfikacja wsp�czynnika zooma (0.1 to arbitralna warto��)
        }
    }


    glColor3f(1.0f, 1.0f, 1.0f);
    // Ustawienie koloru rysowania na bia�y

    glRotatef(theta, 0.0, 1.0, 0.0);  //obr�t obiektu o nowy k�t (x)
    glRotatef(gamma, 1.0, 0.0, 0.0);  //obr�t obiektu o nowy k�t (y)

    if (model == 1)
    {
        Triangle();
        // Narysowanie trojkata
    }
    if (model == 2)
    {
        Pyramid();
        // Narysowanie piramidy
    }
    if (model == 3)
    {
        glTranslatef(0.0f, -4.0f, 0.0f);
        // Przesuniecie obiektu wzgledem osi Y 
        egg_2();
        // Narysowanie jajka (siatka punktow)
    }

    glFlush();
    // Przekazanie polece� rysuj�cych do wykonania

    glutSwapBuffers();
}
/*************************************************************************************/

// Funkcja obslugujaca klawisze klawiatury
void keys(unsigned char key, int x, int y)
{

    if (key == 'q') model = 1;
    if (key == 'w') model = 2;
    if (key == 'e') model = 3;

    if (model == 2)
    {
        if (key == 'z') wall1 = !wall1;
        if (key == 'x') wall2 = !wall2;
        if (key == 'c') wall3 = !wall3;
        if (key == 'v') wall4 = !wall4;
        if (key == 'b') wall5 = !wall5;
    }

    if (key == '1') perspective = 1;
    if (key == '2') perspective = 2;

    RenderScene(); // przerysowanie obrazu sceny
}


/*************************************************************************************/

// Funkcja ustalaj�ca stan renderowania



void MyInit(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Zmienne dla obrazu tekstury
    GLbyte* pBytes;
    GLint ImWidth, ImHeight, ImComponents;
    GLenum ImFormat;

    // Teksturowanie b�dzie prowadzone tyko po jednej stronie �ciany
    glEnable(GL_CULL_FACE);

    //  Przeczytanie obrazu tekstury z pliku o nazwie tekstura.tga
    pBytes = LoadTGAImage("lewuss.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);

    // Zdefiniowanie tekstury 2-D
    glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);

    // Zwolnienie pami�ci
    free(pBytes);

    // W��czenie mechanizmu teksturowania
    glEnable(GL_TEXTURE_2D);

    // Ustalenie trybu teksturowania
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Okre�lenie sposobu nak�adania tekstur
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Definicja materia�u z jakiego zrobiony jest czajnik i jajko
    GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess = { 20.0 };

    // Ustawienie parametr�w materia�u
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    // Definicja �r�d�a �wiat�a (bia�ego)
    GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 8.2414, 5.26967, 1.8681, 1.0 };
    GLfloat att_constant = 1.0;
    GLfloat att_linear = 0.05;
    GLfloat att_quadratic = 0.001;

    // Ustawienie parametr�w �r�d�a �wiat�a
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);

    // Ustawienie opcji systemu o�wietlania sceny
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
}


/*************************************************************************************/

// Funkcja ma za zadanie utrzymanie sta�ych proporcji rysowanych
// w przypadku zmiany rozmiar�w okna.
// Parametry vertical i horizontal (wysoko�� i szeroko�� okna) s� 
// przekazywane do funkcji za ka�dym razem gdy zmieni si� rozmiar okna.

void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
    pix2angle = 360.0 / (float)horizontal;  // przeliczenie pikseli na stopnie (x)
    piy2angle = 360.0 / (float)vertical; // przeliczenie pikseli na stopnie (y)

    glMatrixMode(GL_PROJECTION);
    // Prze��czenie macierzy bie��cej na macierz projekcji

    glLoadIdentity();
    // Czyszcznie macierzy bie��cej

    gluPerspective(70, 1.0, 1.0, 30.0);
    // Ustawienie parametr�w dla rzutu perspektywicznego


    if (horizontal <= vertical)
        glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

    else
        glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
    // Ustawienie wielko�ci okna okna widoku (viewport) w zale�no�ci
    // relacji pomi�dzy wysoko�ci� i szeroko�ci� okna

    glMatrixMode(GL_MODELVIEW);
    // Prze��czenie macierzy bie��cej na macierz widoku modelu  

    glLoadIdentity();
    // Czyszczenie macierzy bie��cej

}

/*************************************************************************************/

// G��wny punkt wej�cia programu. Program dzia�a w trybie konsoli



void main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(300, 300);

    glutCreateWindow("Teksturowanie");

    cout << "> Obrot obiektu [1]" << endl;
    cout << "> Zmiana polozenia obserwatora [2]" << endl;
    cout << "Trojkat [q]" << endl;
    cout << "> Piramida [w]" << endl;
    cout << "> Jajko [e]" << endl << endl;

    cout << "Sterowanie piramida:" << endl;
    cout << "Podstawa [z]" << endl;
    cout << "Przednia sciana [v]" << endl;
    cout << "Tylna sciana [x]" << endl;
    cout << "Prawa sciana [c]" << endl;
    cout << "Lewa sciana [b]" << endl;

    glutKeyboardFunc(keys);
    // Wlaczenie obslugi zdarzen klawiatury

    glutDisplayFunc(RenderScene);
    // Okre�lenie, �e funkcja RenderScene b�dzie funkcj� zwrotn�
    // (callback function).  B�dzie ona wywo�ywana za ka�dym razem
    // gdy zajdzie potrzeba przerysowania okna


    glutReshapeFunc(ChangeSize);
    // Dla aktualnego okna ustala funkcj� zwrotn� odpowiedzialn�
    // za zmiany rozmiaru okna                       


    glutMouseFunc(Mouse);
    // Ustala funkcj� zwrotn� odpowiedzialn� za badanie stanu myszy

    glutMotionFunc(Motion);
    // Ustala funkcj� zwrotn� odpowiedzialn� za badanie ruchu myszy

    MyInit();

    glEnable(GL_DEPTH_TEST);
    // W��czenie mechanizmu usuwania niewidocznych element�w sceny

    glutMainLoop();
    // Funkcja uruchamia szkielet biblioteki GLUT
}

/*************************************************************************************/