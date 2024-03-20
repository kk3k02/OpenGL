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

int perspective = 1;
// Perspektywa obserwatora
// 1 
// 2

const double pi = 3.14159265358979323846;

int N = 30; // Liczba przedzialow boku kwadratu

typedef float point3[3];

static float azimuth = 0.0;
// Inicjalizacja azymutu

static GLfloat elevation = 0.0;

static GLfloat viewer[] = { 0.0, 0.0, 10.0 };
// inicjalizacja położenia obserwatora

static GLfloat theta = 0.0;
// kąt obrotu obiektu wokół osi x

static GLfloat gamma = 0.0;
// kąt obrotu obiektu wokół osi y

static GLfloat pix2angle;
static GLfloat piy2angle;
// przelicznik pikseli na stopnie

static GLint status_left = 0;
static GLint status_right = 0;
// stan klawiszy myszy
// 0 - nie naciśnięto żadnego klawisza
// 1 - naciśnięty zostać lewy klawisz

static int delta_x = 0;
// różnica pomiędzy pozycją bieżącą
// i poprzednią kursora myszy (x)

static int delta_y = 0;
// różnica pomiędzy pozycją bieżącą
// i poprzednią kursora myszy (y)

static int delta_zoom = 0;
// różnica pomiędzy pozycją bieżącą
// i poprzednią zoom

static int x_pos_old = 0;
// poprzednia pozycja kursora myszy (x)

static int y_pos_old = 0;
// poprzednia pozycja kursora myszy (y)

static int zoom_pos_old = 0;
// poprzednia pozycja zoom


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

// Funkcja obliczajaca wartosci katow na podstawie azymutu i elewacji
float calculate_angles(char xyz, float azimuth, float perspecitve)
{
    if (azimuth >= 0 && azimuth <= (2*pi) && perspecitve >= 0 && perspecitve <= (2*pi))
    {
        float result = 0.0;

        if (xyz == 'x') {
            result = delta_zoom * cos(azimuth) * cos(elevation);
        }
        if (xyz == 'y') {
            result = delta_zoom * sin(elevation);
        }
        if (xyz == 'z') {
            result = delta_zoom * sin(azimuth) * cos(elevation);
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

    glBegin(GL_TRIANGLES);

    // Rysowanie tr�jk�t�w jajka
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

            float x = 0.0;
            float y = 0.0;
            float z = 0.0;
            float normalized = 0.0;

            // Pierwszy trojkat
            x = calculate_vector('x', u0, v0);
            y = calculate_vector('y', u0, v0);
            z = calculate_vector('z', u0, v0);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glVertex3f(calculate("x", u0, v0), calculate("y", u0, v0), calculate("z", u0, v0));
            x = calculate_vector('x', u1, v1);
            y = calculate_vector('y', u1, v1);
            z = calculate_vector('z', u1, v1);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glVertex3f(calculate("x", u1, v1), calculate("y", u1, v1), calculate("z", u1, v1));
            x = calculate_vector('x', u2, v2);
            y = calculate_vector('y', u2, v2);
            z = calculate_vector('z', u2, v2);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
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
            glVertex3f(calculate("x", u1, v1), calculate("y", u1, v1), calculate("z", u1, v1));
            x = calculate_vector('x', u3, v3);
            y = calculate_vector('y', u3, v3);
            z = calculate_vector('z', u3, v3);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glVertex3f(calculate("x", u3, v3), calculate("y", u3, v3), calculate("z", u3, v3));
            x = calculate_vector('x', u2, v2);
            y = calculate_vector('y', u2, v2);
            z = calculate_vector('z', u2, v2);
            normalized = normalize_vector(x, y, z);
            x = x / normalized;
            y = y / normalized;
            z = z / normalized;
            glNormal3f(x, y, z);
            glVertex3f(calculate("x", u2, v2), calculate("y", u2, v2), calculate("z", u2, v2));
        }
    }

    glEnd();
}

/*************************************************************************************/

// Funkcja rysująca jajko jako siatkę punktów

void egg_1(void)
{
    float u, v;
    glBegin(GL_LINES);

    // Rysowanie siatki jajka
    for (int i = 0; i < N - 1; i++)
    {
        for (int j = 0; j < N - 1; j++)
        {
            // Pionowe linie siatki
            glVertex3f(calculate("x", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate("y", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate("z", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j));

            glVertex3f(calculate("x", (1.0 / (N - 1)) * (i + 1), (1.0 / (N - 1)) * j),
                calculate("y", (1.0 / (N - 1)) * (i + 1), (1.0 / (N - 1)) * j),
                calculate("z", (1.0 / (N - 1)) * (i + 1), (1.0 / (N - 1)) * j));

            // Poziome linie siatki
            glVertex3f(calculate("x", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate("y", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate("z", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j));

            glVertex3f(calculate("x", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * (j + 1)),
                calculate("y", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * (j + 1)),
                calculate("z", (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * (j + 1)));
        }
    }

    glEnd();
}

/*************************************************************************************/

// Funkcja rysująca osie układu wspó?rz?dnych
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
// Funkcja "bada" stan myszy i ustawia wartości odpowiednich zmiennych globalnych

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
// Funkcja "monitoruje" położenie kursora myszy i ustawia wartości odpowiednich
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

// Funkcja określająca co ma być rysowane (zawsze wywoływana, gdy trzeba
// przerysować scenę)
void RenderScene(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Czyszczenie okna aktualnym kolorem czyszczącym

    glLoadIdentity();
    // Czyszczenie macierzy bie??cej

    gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // Zdefiniowanie położenia obserwatora
    Axes();
    // Narysowanie osi przy pomocy funkcji zdefiniowanej powyżej

    if (perspective == 1) {
        if (status_left == 1 && status_right == 0)
        {
            theta += delta_x * pix2angle;
            gamma += delta_y * piy2angle;
        }
        if (status_right == 1 && status_left == 0)
        {
            viewer[2] += delta_zoom * pix2angle * 0.1; // Modyfikacja współczynnika zooma (0.1 to arbitralna wartość)
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
            viewer[2] += delta_zoom * pix2angle * 0.1; // Modyfikacja współczynnika zooma (0.1 to arbitralna wartość)
        }
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    // Ustawienie koloru rysowania na biały

    glRotatef(theta, 0.0, 1.0, 0.0);  //obrót obiektu o nowy kąt (x)
    glRotatef(gamma, 1.0, 0.0, 0.0);  //obrót obiektu o nowy kąt (y)

    if (model == 1)
    {
        glutWireTeapot(3.0);
        // Narysowanie czajnika (wire)
    }
    if (model == 2)
    {
        glutSolidTeapot(3.0);
        // Narysowanie czajnika (solid)
    }
    if (model == 3)
    {
        glTranslatef(0.0f, -4.0f, 0.0f);
        // Przesuniecie obiektu wzgledem osi Y 
        egg_1();
        // Narysowanie jajka (siatka punktow)
    } 
    if (model == 4)
    {
        glTranslatef(0.0f, -4.0f, 0.0f);
        // Przesuniecie obiektu wzgledem osi Y 
        egg_2();
        // Narysowanie jajka (siatka punktow)
    }

    glFlush();
    // Przekazanie poleceń rysujących do wykonania

    glutSwapBuffers();
}
/*************************************************************************************/

// Funkcja obslugujaca klawisze klawiatury
void keys(unsigned char key, int x, int y)
{
    if (key == 'q') model = 1;
    if (key == 'w') model = 2;
    if (key == 'e') model = 3;
    if (key == 'r') model = 4;
    if (key == '1') perspective = 1;
    if (key == '2') perspective = 2;

    RenderScene(); // przerysowanie obrazu sceny
}

/*************************************************************************************/

// Funkcja ustalająca stan renderowania



void MyInit(void)
{

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Kolor czyszczący (wypełnienia okna) ustawiono na czarny


    /*************************************************************************************/

//  Definicja materiału z jakiego zrobiony jest czajnik
//  i definicja źródła światła

/*************************************************************************************/


/*************************************************************************************/
// Definicja materiału z jakiego zrobiony jest czajnik

    GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    // współczynniki ka =[kar,kag,kab] dla światła otoczenia

    GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    // współczynniki kd =[kdr,kdg,kdb] światła rozproszonego

    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    // współczynniki ks =[ksr,ksg,ksb] dla światła odbitego               

    GLfloat mat_shininess = { 20.0 };
    // współczynnik n opisujący połysk powierzchni


/*************************************************************************************/
// Definicja źródła światła

    // Definicja źródła światła (kolor żółty)

    GLfloat light_position[] = { 0.0, 0.0, 10.0, 1.0 };
    // położenie źródła


    GLfloat light_ambient[] = { 0.2, 0.2, 0.0, 1.0 };
    // składowe intensywności świecenia źródła światła otoczenia
    // Ia = [Iar,Iag,Iab]

    GLfloat light_diffuse[] = { 1.0, 1.0, 0.0, 1.0 };
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie dyfuzyjne Id = [Idr,Idg,Idb]

    GLfloat light_specular[] = { 1.0, 1.0, 0.0, 1.0 };
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie kierunkowe Is = [Isr,Isg,Isb]

    GLfloat att_constant = { 1.0 };
    // składowa stała ds dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

    GLfloat att_linear = { 0.05 };
    // składowa liniowa dl dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

    GLfloat att_quadratic = { 0.001 };
    // składowa kwadratowa dq dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

/*************************************************************************************/
// Ustawienie parametrów materiału i źródła światła

/*************************************************************************************/
// Ustawienie patrametrów materiału


    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);


    /*************************************************************************************/
    // Ustawienie parametrów źródła

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);


    /*************************************************************************************/
    // Ustawienie opcji systemu oświetlania sceny

    glShadeModel(GL_SMOOTH); // właczenie łagodnego cieniowania
    glEnable(GL_LIGHTING);   // właczenie systemu oświetlenia sceny
    glEnable(GL_LIGHT0);     // włączenie źródła o numerze 0
    glEnable(GL_DEPTH_TEST); // włączenie mechanizmu z-bufora

    /*************************************************************************************/
}

/*************************************************************************************/


// Funkcja ma za zadanie utrzymanie stałych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokość i szerokość okna) są 
// przekazywane do funkcji za każdym razem gdy zmieni się rozmiar okna.



void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
    pix2angle = 360.0 / (float)horizontal;  // przeliczenie pikseli na stopnie (x)
    piy2angle = 360.0 / (float)vertical; // przeliczenie pikseli na stopnie (y)

    glMatrixMode(GL_PROJECTION);
    // Przełączenie macierzy bieżącej na macierz projekcji

    glLoadIdentity();
    // Czyszcznie macierzy bieżącej

    gluPerspective(70, 1.0, 1.0, 30.0);
    // Ustawienie parametrów dla rzutu perspektywicznego


    if (horizontal <= vertical)
        glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

    else
        glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
    // Ustawienie wielkości okna okna widoku (viewport) w zależności
    // relacji pomiędzy wysokością i szerokością okna

    glMatrixMode(GL_MODELVIEW);
    // Przełączenie macierzy bieżącej na macierz widoku modelu  

    glLoadIdentity();
    // Czyszczenie macierzy bieżącej

}

/*************************************************************************************/

// Główny punkt wejścia programu. Program działa w trybie konsoli



void main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(300, 300);

    glutCreateWindow("Oświetlenie");


    cout << "Perspektywa [1]: 1" << endl;
    cout << "Perspektywa [2]: 2" << endl;
    cout << "Model 1 (Czajnik [wire]): q" << endl;
    cout << "Model 2 (Czajnik [solid]): w" << endl;
    cout << "Model 3 (Jajko [siatka]): e" << endl;
    cout << "Model 3 (Jajko [trojkaty]): r" << endl;

    glutKeyboardFunc(keys);
    // Wlaczenie obslugi zdarzen klawiatury

    glutDisplayFunc(RenderScene);
    // Określenie, że funkcja RenderScene będzie funkcją zwrotną
    // (callback function).  Będzie ona wywoływana za każdym razem
    // gdy zajdzie potrzeba przerysowania okna


    glutReshapeFunc(ChangeSize);
    // Dla aktualnego okna ustala funkcję zwrotną odpowiedzialną
    // za zmiany rozmiaru okna                       


    glutMouseFunc(Mouse);
    // Ustala funkcję zwrotną odpowiedzialną za badanie stanu myszy

    glutMotionFunc(Motion);
    // Ustala funkcję zwrotną odpowiedzialną za badanie ruchu myszy

    MyInit();

    glEnable(GL_DEPTH_TEST);
    // Włączenie mechanizmu usuwania niewidocznych elementów sceny

    glutMainLoop();
    // Funkcja uruchamia szkielet biblioteki GLUT
}

/*************************************************************************************/
