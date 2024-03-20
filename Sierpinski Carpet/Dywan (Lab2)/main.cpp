#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <iostream>

using namespace std;

int recursion = 0;
int blur_level = 0;
bool drawn = false; // Flaga œledz¹ca, czy dywan zosta³ ju¿ narysowany

// Funkcja do generowania liczb pseudolosowych
int random(int blur_level) {
    if (blur_level == 0 || blur_level == 1) return 1;
    return rand() % blur_level;
}

// Funkcja do rysowania dywanu Sierpiñskiego
void drawCarpet(int x, int y, int size, int depth, int blur_level) {
    if (depth == 0) {

        glColor3f(random(blur_level), random(blur_level), random(blur_level));
        // Ustawienie aktualnego koloru rysowania na losowy

        // Losowanie poziomu rozmycia
        int blur = 0;
        if (blur_level != 0) blur = random(blur_level);


        // Narysuj czworok¹t
        glBegin(GL_POLYGON);
        glVertex2i(x - blur, y - blur);
        glVertex2i(x + size + blur, y + blur);
        glVertex2i(x + size - blur, y + size - blur);
        glVertex2i(x + blur, y + size + blur);
        glEnd();
    }
    else {
        int newSize = size / 3;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (!(i == 1 && j == 1)) {
                    int newX = x + i * newSize;
                    int newY = y + j * newSize;
                    // Rekurencyjnie rysuj mniejsze dywany
                    drawCarpet(newX, newY, newSize, depth - 1, blur_level);
                }
            }
        }
    }

    glFlush();
}

// Funkcja okreœlaj¹ca, co ma byæ rysowane
// (zawsze wywo³ywana, gdy trzeba przerysowaæ scenê)
void RenderScene(void) {
    if (!drawn) {
        int size = powf(3, recursion);

        glClear(GL_COLOR_BUFFER_BIT); // Wyczyœæ bufor tylko raz

        // Rozpocznij rysowanie dywanu od punktu (-100, -100)
        drawCarpet(-100.0f, -100.0f, size, recursion, blur_level);

        drawn = true; // Oznacz, ¿e dywan zosta³ ju¿ narysowany
    }

    glFlush();
}

// Funkcja ustalaj¹ca stan renderowania
void MyInit(void) {
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    // Kolor okna wnêtrza okna - ustawiono na szary
}

// Funkcja s³u¿¹ca do kontroli zachowania proporcji rysowanych obiektów
// niezale¿nie od rozmiarów okna graficznego
void ChangeSize(GLsizei horizontal, GLsizei vertical) {
    GLfloat AspectRatio;

    if (vertical == 0)
        vertical = 1;

    glViewport(0, 0, horizontal, vertical);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;

    cout << "Podaj poziom rekursji: ";
    cin >> recursion;

    cout << "\nPodaj poziom perturbacji: ";
    cin >> blur_level;

    // Oblicz now¹ wartoœæ `size` na podstawie wzoru size = powf(3, recursion) - 100.00
    float size = powf(3, recursion) - 100.0;

    if (horizontal <= vertical)
        glOrtho(-100.0, size, -100.0 / AspectRatio, size / AspectRatio, 1.0, -1.0);
    else
        glOrtho(-100.0 * AspectRatio, size * AspectRatio, -100.0, size, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// G³ówny punkt wejœcia programu. Program dzia³a w trybie konsoli
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    // Funkcja MyInit (zdefiniowana powy¿ej) wykonuje wszelkie 
    // inicjalizacje konieczne przed przyst¹pieniem do renderowania

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    // Ustawienie trybu wyœwietlania
    // GLUT_SINGLE - pojedynczy bufor wyœwietlania
    // GLUT_RGBA - model kolorów RGB

    glutCreateWindow("Dywan Sierpiñskiego z perturbacjami");
    // Utworzenie okna i okreœlenie treœci napisu w nag³ówku okna

    glutDisplayFunc(RenderScene);
    // Okreœlenie, ¿e funkcja RenderScene bêdzie funkcj¹ zwrotn¹ (callback)
    // Biblioteka GLUT bêdzie wywo³ywa³a t¹ funkcjê za ka¿dym razem, gdy
    // trzeba bêdzie przerysowaæ okno

    glutReshapeFunc(ChangeSize);
    // Dla aktualnego okna ustala funkcjê zwrotn¹ odpowiedzialn¹ za
    // zmiany rozmiaru okna

    glutMainLoop();
    // Funkcja uruchamia szkielet biblioteki GLUT

    return 0;
}
