/*************************************************************************************/

//  Szkielet programu do tworzenia modelu sceny 3-D z wizualizacj¹ osi 
//  uk³adu wspó³rzednych

/*************************************************************************************/

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <math.h>
#include "main.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

const double pi = 3.14159265358979323846;

typedef float point3[3];

int N = 20; // Iloœæ przedzia³ów boku kwadratu

int model = 1;  // 1- punkty, 2- siatka, 3 - wype³nione trójk¹ty

static GLfloat theta[] = { 0.0, 0.0, 0.0 }; // trzy k¹ty obrotu

/*************************************************************************************/

// Funkcja obliczaj¹ca wspó³rzêdne powierzchni opisanej równaniami parametrycznymi

float calculate(char xyz, float u, float v)
{
    float result;

    if (u >= 0 && u <= 1 && v >= 0 && v <= 1)
    {
        // x(u, v)
        if (xyz == 'x')
        {
            result = (-90.0 * powf(u, 5) + 225.0 * powf(u, 4) - 270.0 * powf(u, 3) + 180.0 * powf(u, 2) - 45.0 * u) * cos(float(pi) * v);
        }
        // y(u, v)
        else if (xyz == 'y')
        {
            result = (160.0 * powf(u, 4) - 320.0 * powf(u, 3) + 160 * powf(u, 2));
        }
        // z(u, v)
        else if (xyz == 'z')
        {
            result = (-90.0 * powf(u, 5) + 225.0 * powf(u, 4) - 270.0 * powf(u, 3) + 180.0 * powf(u, 2) - 45.0 * u) * sin(float(pi) * v);
        }

        return result;
    }

    return NULL;
}

/*************************************************************************************/

// Funkcja rysuj¹ca chmurê punktów

void model_1(void)
{
    point3* points;
    points = new point3[N*N];

    float u, v;

    glBegin(GL_POINTS);

    for (int i = 0; i < N; i++)
    {
        u = (1.0 / N) * (i + 1);

        for (int j = 0; j < N; j++)
        {
            v = (1.0 / N) * (j + 1);

            points[(N * i) + j][0] = { calculate('x', u, v) };
            points[(N * i) + j][1] = { calculate('y', u, v) };
            points[(N * i) + j][2] = { calculate('z', u, v) };

            glVertex3fv(points[(N * i) + j]);
        }
    }

    glEnd();

    delete[] points;
}

/*************************************************************************************/

// Funkcja rysuj¹ca siatkê punktów

void model_2(void)
{
    float u, v;
    glBegin(GL_LINES);

    // Rysowanie siatki jajka
    for (int i = 0; i < N - 1; i++)
    {
        for (int j = 0; j < N - 1; j++)
        {
            // Pionowe linie siatki
            glVertex3f(calculate('x', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate('y', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate('z', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j));

            glVertex3f(calculate('x', (1.0 / (N - 1)) * (i + 1), (1.0 / (N - 1)) * j),
                calculate('y', (1.0 / (N - 1)) * (i + 1), (1.0 / (N - 1)) * j),
                calculate('z', (1.0 / (N - 1)) * (i + 1), (1.0 / (N - 1)) * j));

            // Poziome linie siatki
            glVertex3f(calculate('x', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate('y', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j),
                calculate('z', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * j));

            glVertex3f(calculate('x', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * (j + 1)),
                calculate('y', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * (j + 1)),
                calculate('z', (1.0 / (N - 1)) * i, (1.0 / (N - 1)) * (j + 1)));
        }
    }

    glEnd();
}

/*************************************************************************************/

// Funkcja rysuj¹ca obiekt w postaci wype³nionych trójk¹tów

void model_3(void)
{
    // Inicjalizacja generatora liczb pseudolosowych
    srand(static_cast<unsigned int>(time(nullptr)));

    glBegin(GL_TRIANGLES);

    // Rysowanie trójk¹tów jajka
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

            // Losowy kolor
            glColor3f(static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX);

            // Pierwszy trójk¹t
            glVertex3f(calculate('x', u0, v0), calculate('y', u0, v0), calculate('z', u0, v0));
            glVertex3f(calculate('x', u1, v1), calculate('y', u1, v1), calculate('z', u1, v1));
            glVertex3f(calculate('x', u2, v2), calculate('y', u2, v2), calculate('z', u2, v2));

            // Losowy kolor
            glColor3f(static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX,
                static_cast<float>(rand()) / RAND_MAX);

            // Drugi trójk¹t
            glVertex3f(calculate('x', u1, v1), calculate('y', u1, v1), calculate('z', u1, v1));
            glVertex3f(calculate('x', u3, v3), calculate('y', u3, v3), calculate('z', u3, v3));
            glVertex3f(calculate('x', u2, v2), calculate('y', u2, v2), calculate('z', u2, v2));
        }
    }

    glEnd();
}

/*************************************************************************************/

// Funkcja obracaj¹ca jajko

void spinEgg()
{

    theta[0] -= 0.5;
    if (theta[0] > 360.0) theta[0] -= 360.0;

    theta[1] -= 0.5;
    if (theta[1] > 360.0) theta[1] -= 360.0;

    theta[2] -= 0.5;
    if (theta[2] > 360.0) theta[2] -= 360.0;

    glutPostRedisplay(); // Odœwie¿enie zawartoœci aktualnego okna
}


/*************************************************************************************/

// Funkcja rysuj¹ca osie uk³adu wspó³rzêdnych



void Axes(void)
{

    point3  x_min = { -5.0, 0.0, 0.0 };
    point3  x_max = { 5.0, 0.0, 0.0 };
    // pocz¹tek i koniec obrazu osi x

    point3  y_min = { 0.0, -5.0, 0.0 };
    point3  y_max = { 0.0,  5.0, 0.0 };
    // pocz¹tek i koniec obrazu osi y

    point3  z_min = { 0.0, 0.0, -5.0 };
    point3  z_max = { 0.0, 0.0,  5.0 };
    //  pocz¹tek i koniec obrazu osi y
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

// Funkcja okreœlaj¹ca co ma byæ rysowane (zawsze wywo³ywana gdy trzeba
// przerysowaæ scenê)



void RenderScene(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Czyszczenie okna aktualnym kolorem czyszcz¹cym

    glLoadIdentity();
    // Czyszczenie macierzy bie¿¹cej
    Axes();
    // Narysowanie osi przy pomocy funkcji zdefiniowanej wy¿ej
    glColor3f(0.0f, 1.0f, 0.0f); 
    // Ustawienie koloru rysowania na bia³y
    glRotatef(theta[0], 1.0, 0.0, 0.0);

    glRotatef(theta[1], 0.0, 1.0, 0.0);

    glRotatef(theta[2], 0.0, 0.0, 1.0);

    glTranslatef(0.0f, -4.0f, 0.0f);
    // Przesuniêcie obiektu wzglêdem osi Y 
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f);  
    // Obrót o 30 stopni

    // Rysowanie modeli
    if (model == 1)
    {
        model_1();
        // Rysowanie chmury punktów
    }
    if (model == 2)
    {
        model_2();
        // Rysowanie siatki punktów
    }
    if (model == 3)
    {
        model_3();
        // Rysowanie jajka z³o¿onego z kolorowych trójk¹tów
    }
    
    glFlush();
    // Przekazanie poleceñ rysuj¹cych do wykonania


    glutSwapBuffers();
    //
}

/*************************************************************************************/

// Funkcja ustalaj¹ca stan renderowania



void MyInit(void)
{

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Kolor czyszc¹cy (wype³nienia okna) ustawiono na czarny

}

/*************************************************************************************/

// Funkcja ma za zadanie utrzymanie sta³ych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokoœæ i szerokoœæ okna) s¹
// przekazywane do funkcji za ka¿dym razem gdy zmieni siê rozmiar okna.



void ChangeSize(GLsizei horizontal, GLsizei vertical)
{

    GLfloat AspectRatio;
    // Deklaracja zmiennej AspectRatio  okreœlaj¹cej proporcjê
    // wymiarów okna 
    if (vertical == 0)  // Zabezpieczenie przed dzieleniem przez 0
        vertical = 1;
    glViewport(0, 0, horizontal, vertical);
    // Ustawienie wielkoœciokna okna widoku (viewport)
    // W tym przypadku od (0,0) do (horizontal, vertical)  
    glMatrixMode(GL_PROJECTION);
    // Prze³¹czenie macierzy bie¿¹cej na macierz projekcji 
    glLoadIdentity();
    // Czyszcznie macierzy bie¿¹cej            
    AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;
    // Wyznaczenie wspó³czynnika  proporcji okna
    // Gdy okno nie jest kwadratem wymagane jest okreœlenie tak zwanej
    // przestrzeni ograniczaj¹cej pozwalaj¹cej zachowaæ w³aœciwe
    // proporcje rysowanego obiektu.
    // Do okreslenia przestrzeni ograniczj¹cej s³u¿y funkcja
    // glOrtho(...)            
    if (horizontal <= vertical)

        glOrtho(-7.5, 7.5, -7.5 / AspectRatio, 7.5 / AspectRatio, 10.0, -10.0);
    else

        glOrtho(-7.5 * AspectRatio, 7.5 * AspectRatio, -7.5, 7.5, 10.0, -10.0);
    glMatrixMode(GL_MODELVIEW);
    // Prze³¹czenie macierzy bie¿¹cej na macierz widoku modelu                                   

    glLoadIdentity();
    // Czyszcenie macierzy bie¿¹cej
}

/*************************************************************************************/

// Funkcja obs³uguj¹ca klawisze klawiatury
void keys(unsigned char key, int x, int y)
{
    if (key == 'q') model = 1;
    if (key == 'w') model = 2;
    if (key == 'e') model = 3;

    RenderScene(); // przerysowanie obrazu sceny
}

/*************************************************************************************/

// G³ówny punkt wejœcia programu. Program dzia³a w trybie konsoli



void main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(300, 300);

    glutCreateWindow("Uk³ad wspó³rzêdnych 3-D");

    cout << "Model 1 (Chmura punktow): q" << endl;
    cout << "Model 2 (Siatka punktow): w" << endl;
    cout << "Model 3 (Kolorowe trojkaty): e" << endl;

    glutKeyboardFunc(keys);
    // W³¹czenie obs³ugi zdarzeñ klawiatury
    glutDisplayFunc(RenderScene);
    // Okreœlenie, ¿e funkcja RenderScene bêdzie funkcj¹ zwrotn¹
    // (callback function).  Bedzie ona wywo³ywana za ka¿dym razem
    // gdy zajdzie potrzba przeryswania okna 
    glutReshapeFunc(ChangeSize);
    // Dla aktualnego okna ustala funkcjê zwrotn¹ odpowiedzialn¹
    // zazmiany rozmiaru okna      
    MyInit();
    // Funkcja MyInit() (zdefiniowana powy¿ej) wykonuje wszelkie
    // inicjalizacje konieczne  przed przyst¹pieniem do renderowania
    glEnable(GL_DEPTH_TEST);
    // W³¹czenie mechanizmu usuwania powierzchni niewidocznych
    glutIdleFunc(spinEgg);
    // W³¹czenie obracania siê jajka
    glutMainLoop();
    // Funkcja uruchamia szkielet biblioteki GLUT
}

/*************************************************************************************/