#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <tuple>
#include <typeinfo>
#include <time.h>
#include <SFML/Graphics.hpp>
#include <omp.h>
#include <string>
#include <map>
using namespace std;


// Тут переменные необходимые для начальной генерации (самой первой) x_min, x_max, j_min, j_max.
// Так же масив INIT_PAL для создания градиента по 5 цветам, с шагом 2048. То есть от первого цвета идет градиент до второго с шагом 512 и так далее.
// Для реализации зума нужно перевести коордниатную плоскость (расчетную) в координатную плоскость окна,
// для этого нужно знать расстояние от центра координат до центра окна (левый верхни угол).
// Это расстояние вычисляется по x_min, x_max, j_min, j_max. Писал код 2 недели, мог управиться за 5 дней если бы писал каждый день.
// Пока не знаю ООП писал через функции. Использовал: таплы, заданные массивы , switch case. 
int LENGTH = 1200;
int HEIGHT = 600;  
long double zoom_scale = 1;
double x_min = -2;
double x_max = 1;
double j_min = -1;
double j_max = 1;
int n_iter = 128;
tuple<double, double, double> INIT_PAL[5] = 
{
    make_tuple(0, 7, 100),
    make_tuple(32, 107, 203),
    make_tuple(237, 255, 255),
    make_tuple(255, 170, 0),
    make_tuple(0, 2, 0)
}; 
tuple<double, double, double> PALETTE[2048];
int PALETTE_INDEX = 0;


void palette(tuple<double, double, double> from, tuple<double, double, double> to)
{
    PALETTE[PALETTE_INDEX] = from;
    PALETTE_INDEX ++;
    double r_from, g_from, b_from;
    double r_to, g_to, b_to;
    tie(r_from, g_from, b_from) = from;
    tie(r_to, g_to, b_to) = to;
    double r_dif, g_dif, b_dif;
    r_dif = (r_to > r_from ? r_to : r_from) - (r_to < r_from ? r_to : r_from);
    g_dif = (g_to > g_from ? g_to : g_from) - (g_to < g_from ? g_to : g_from);
    b_dif = (b_to > b_from ? b_to : b_from) - (b_to < b_from ? b_to : b_from);
    double r_c, g_c, b_c;
    for (int i = 1; i < 512; i++)
    {
        r_c = r_from = r_from < r_to ? (r_from + r_dif/512) : (r_from - r_dif/512);
        g_c = g_from = g_from < g_to ? (g_from + g_dif/512) : (g_from - g_dif/512);
        b_c = b_from = b_from < b_to ? (b_from + b_dif/512) : (b_from - b_dif/512);
        PALETTE[PALETTE_INDEX] = make_tuple(r_c, g_c, b_c);
        PALETTE_INDEX ++;
    } 
}


double len(double max, double min)
{
    if (min > 0)
    {
        return max-min;
    }
    else if (max < 0) 
    {
        return abs(min) + max;
    }
    else
    {
        return abs(min) + max;
    }
}


int main() {
    for (int i = 0; i < 4; i++)
    {
        palette(INIT_PAL[i], INIT_PAL[i+1]);
    }
    clock_t tStart = clock();
    sf::Text iter_text;
    sf::Text zoom_text;
    sf::Font font;
    font.loadFromFile("Font_regular.ttf");
    iter_text.setFont(font);
    zoom_text.setFont(font);
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Image image;
    sf::RenderWindow window(sf::VideoMode(LENGTH, HEIGHT), "Mandelbrot Set");
    window.clear(sf::Color::Black);
    while (window.isOpen())
    {
        image.create(LENGTH, HEIGHT, sf::Color::Black);
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed){
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                    {
                        sf::Vector2i position = sf::Mouse::getPosition(window);
                        double len_of_x = len(x_max, x_min);
                        double len_of_j = len(j_max, j_min);
                        zoom_scale *= 2;
                        double x_max_new = (position.x*len_of_x/LENGTH + x_min) + (len_of_x/(4));
                        double x_min_new = (position.x*len_of_x/LENGTH + x_min) - (len_of_x/(4));
                        double j_max_new = (-position.y*len_of_j/HEIGHT + j_max) + (len_of_j/(4));
                        double j_min_new = (-position.y*len_of_j/HEIGHT + j_max) - (len_of_j/(4));
                        x_max = x_max_new;
                        x_min = x_min_new;
                        j_max = j_max_new;
                        j_min = j_min_new;
                        cout << zoom_scale << endl;
                    }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                    {
                        sf::Vector2i position = sf::Mouse::getPosition();
                        double len_of_x = len(x_max, x_min);
                        double len_of_j = len(j_max, j_min);
                        zoom_scale /= 2;
                        double x_max_new = x_max + (len_of_x/2);
                        double x_min_new = x_min - (len_of_x/2);
                        double j_max_new = j_max + (len_of_x/2);
                        double j_min_new = j_min - (len_of_x/2);
                        x_max = x_max_new;
                        x_min = x_min_new;
                        j_max = j_max_new;
                        j_min = j_min_new;
                        cout << zoom_scale << endl;
                    }
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                {
                    n_iter *= 2;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                {
                    n_iter /= 2;
                }
            }
    }
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < LENGTH; x++) 
        {
            for (int j = 0; j < HEIGHT; j++) 
            {
                complex<double> z(0.0, 0.0);
                complex<double> c((x*len(x_max, x_min)/LENGTH) + x_min, \
                (j*len(j_max,j_min)/HEIGHT) - j_max);
                double i;
                for (i = 1; i < n_iter + 1; i++)
                {
                        z = z*z + c;
                        if (abs(z) > 2)
                        {
                            break;
                        }
                }
                double index = (i/n_iter)*2047;
                int r, g, b;
                tie(r, g, b) = PALETTE[int(index)];
                image.setPixel(x, j, sf::Color(r, g, b));
                }
            }
    iter_text.setString("Number of iteration: " + to_string(n_iter));
    zoom_text.setString("Zoom scale: " + to_string(zoom_scale));
    iter_text.setCharacterSize(14);
    zoom_text.setCharacterSize(14);
    iter_text.setPosition(0, 0);
    zoom_text.setPosition(0.0, 15);
    iter_text.setFillColor(sf::Color::White);
    zoom_text.setFillColor(sf::Color::White);
    texture.loadFromImage(image);
    sprite.setTexture(texture);
    window.draw(sprite);
    window.draw(iter_text);
    window.draw(zoom_text);
    window.display();
    image.saveToFile("Mandleset.png");
    }    
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    return 0;
}
