#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Menu.h"
#include "tablero.h"
#include "Arena.h"
#include <SFML/Audio.hpp>

enum class EstadoJuego {
    MENU,
    TABLERO,
    ARENA_COMBATE,
    FIN_PARTIDA,
    SALIR
};

class Coordinador {
private:
    EstadoJuego estado;
    Menu        menu;
    Tablero     tablero;
    Arena       arenaVisual;

    // Piezas en combate actual
    Pieza* piezaAzulCombate = nullptr;
    Pieza* piezaRojaCombate = nullptr;

    // Selección en el tablero
    int selFila = -1, selCol = -1;

    // Teleport: necesita dos clics (origen y destino)
    bool modoTeleport = false;
    int  teleportOrigenFila = -1;
    int  teleportOrigenCol = -1;


    // Combate en tiempo real
    sf::Clock    relojCombate;
    sf::Vector2f posAzul, posRoja;
    float        velAzul = 200.f, velRoja = 200.f;
    float        recargaAzul = 0.f, recargaRoja = 0.f;
    int          vidaAzulCombate = 0, vidaRojaCombate = 0;
public:
    struct Proyectil {
        sf::CircleShape forma;
        sf::Vector2f    vel;
        int             danio;
        bool            esAzul;
        bool            activo = true;
        bool            atraviesaObstaculos = false;
        bool            esExplosion = false;
        float           radioExplosion = 0.f;
    };
    std::vector<Proyectil> proyectiles;
    private:
    struct Obstaculo {
        sf::Sprite sprite;
        sf::FloatRect hitbox;
    };
    std::vector<Obstaculo> obstaculos;
    sf::Texture texObstaculo;

    sf::Font fuente;
    bool     fuenteCargada = false;
    std::string ganadorTexto = "";

    //añade los sonidos
    sf::SoundBuffer bufferMuerte;
    sf::Sound       sonidoMuerte;

    sf::SoundBuffer bufferMenu;
    sf::Sound       sonidoMenu;

    void procesarClickTablero(int px, int py);
    void iniciarCombate();
    void actualizarCombate(float dt);
    void dibujarCombate(sf::RenderWindow& window);
    void dispararAzul();
    void dispararRoja();
    bool ganoAzul = false;
    void reiniciar();

public:
    Coordinador();
    ~Coordinador() = default;

    void inicializar();
    void gestionarEventos(sf::RenderWindow& window, sf::Event& event);
    void actualizar(sf::RenderWindow& window);
    void dibujar(sf::RenderWindow& window);
    bool salir();
};