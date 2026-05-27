#include "tablero.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <map>

// La ventana se pasa desde el coordinador
// Para dibujar usamos una referencia externa
// IMPORTANTE: el coordinador llama a tablero.dibuja() y luego dibuja la selección encima

// Constantes visuales (deben coincidir con coordinador.cpp)
static constexpr float TAM_CASILLA = 70.f;
static constexpr float OFFSET_X = 100.f;
static constexpr float OFFSET_Y = 50.f;

// Referencia a la ventana SFML (definida en Main.cpp)
extern sf::RenderWindow* gVentana;

// =========================================================
// CONSTRUCTOR / DESTRUCTOR
// =========================================================
Tablero::Tablero() {
    turnoActual = Equipo::Azul;
    contadorTurnos = 0;
    inicializa();
}

Tablero::~Tablero() {
    for (int i = 0; i < TAM; ++i)
        for (int j = 0; j < TAM; ++j)
            if (matriz[i][j].pieza) { delete matriz[i][j].pieza; matriz[i][j].pieza = nullptr; }
}

// =========================================================
// INICIALIZA - coloca todas las piezas
// =========================================================
void Tablero::inicializa() {
    for (int i = 0; i < TAM; ++i)
        for (int j = 0; j < TAM; ++j) {
            if (matriz[i][j].pieza) { delete matriz[i][j].pieza; matriz[i][j].pieza = nullptr; }
            matriz[i][j].esPuntoPoder = false;
        }

    // Puntos de poder
    for (auto& p : std::vector<std::pair<int, int>>{ {4,4},{0,4},{8,4},{4,0},{4,8} })
        matriz[p.first][p.second].esPuntoPoder = true;

    // --- Bando Azul ---
    // Columna 0 (piezas especiales)
    matriz[0][0].pieza = new Valkiria(Equipo::Azul);
    matriz[1][0].pieza = new Golem(Equipo::Azul);
    matriz[2][0].pieza = new Dragon(Equipo::Azul);
    matriz[3][0].pieza = new Curandera(Equipo::Azul);
    matriz[4][0].pieza = new Mago(Equipo::Azul);
    matriz[5][0].pieza = new Murcielago(Equipo::Azul);
    matriz[6][0].pieza = new Dragon(Equipo::Azul);
    matriz[7][0].pieza = new Golem(Equipo::Azul);
    matriz[8][0].pieza = new Valkiria(Equipo::Azul);

    // Columna 1 (caballeros)
    matriz[0][1].pieza = new Arqueras(Equipo::Azul);
    matriz[1][1].pieza = new Caballero(Equipo::Azul);
    matriz[2][1].pieza = new Caballero(Equipo::Azul);
    matriz[3][1].pieza = new Caballero(Equipo::Azul);
    matriz[4][1].pieza = new Caballero(Equipo::Azul);
    matriz[5][1].pieza = new Caballero(Equipo::Azul);
    matriz[6][1].pieza = new Caballero(Equipo::Azul);
    matriz[7][1].pieza = new Caballero(Equipo::Azul);
    matriz[8][1].pieza = new Arqueras(Equipo::Azul);

    // --- Bando Rojo ---
    // Columna 8 (piezas especiales)
    matriz[0][8].pieza = new Bandida(Equipo::Rojo);
    matriz[1][8].pieza = new PEKKA(Equipo::Rojo);
    matriz[2][8].pieza = new Dragon_infernal(Equipo::Rojo);
    matriz[3][8].pieza = new Esbirro(Equipo::Rojo);
    matriz[4][8].pieza = new Bruja(Equipo::Rojo);
    matriz[5][8].pieza = new Dragon_electrico(Equipo::Rojo);
    matriz[6][8].pieza = new Dragon_infernal(Equipo::Rojo);
    matriz[7][8].pieza = new PEKKA(Equipo::Rojo);
    matriz[8][8].pieza = new Bandida(Equipo::Rojo);

    // Columna 7 (caballeros oscuros)
    matriz[0][7].pieza = new Reina_arquera(Equipo::Rojo);
    matriz[1][7].pieza = new Caballero_oscuro(Equipo::Rojo);
    matriz[2][7].pieza = new Caballero_oscuro(Equipo::Rojo);
    matriz[3][7].pieza = new Caballero_oscuro(Equipo::Rojo);
    matriz[4][7].pieza = new Caballero_oscuro(Equipo::Rojo);
    matriz[5][7].pieza = new Caballero_oscuro(Equipo::Rojo);
    matriz[6][7].pieza = new Caballero_oscuro(Equipo::Rojo);
    matriz[7][7].pieza = new Caballero_oscuro(Equipo::Rojo);
    matriz[8][7].pieza = new Reina_arquera(Equipo::Rojo);
}

// =========================================================
// DIBUJA
// =========================================================
void Tablero::dibuja() const {
    if (!gVentana) return;
    // Fondo del tablero
    static sf::Texture texFondo;
    static sf::Sprite sprFondo;
    static bool cargado = false;
    if (!cargado) {
        texFondo.loadFromFile("assets/arena_fondo.png");
        sprFondo.setTexture(texFondo);
        sprFondo.setScale(
            1280.f / texFondo.getSize().x,
            720.f / texFondo.getSize().y
        );
        cargado = true;
    }
    gVentana->draw(sprFondo);
    sf::RenderWindow& window = *gVentana;

    for (int i = 0; i < TAM; ++i) {
        for (int j = 0; j < TAM; ++j) {
            // Casilla
            static sf::Texture texAzul, texRojo;
            static bool texCargadas = false;
            if (!texCargadas) {
                texAzul.loadFromFile("assets/tile_azul.png");
                texRojo.loadFromFile("assets/tile_rojo.png");
                texCargadas = true;
            }

            sf::Sprite casillaSprite;
            casillaSprite.setTexture((i + j) % 2 == 0 ? texAzul : texRojo);
            casillaSprite.setPosition(OFFSET_X + j * TAM_CASILLA, OFFSET_Y + i * TAM_CASILLA);
            float scaleX = TAM_CASILLA / casillaSprite.getTexture()->getSize().x;
            float scaleY = TAM_CASILLA / casillaSprite.getTexture()->getSize().y;
            casillaSprite.setScale(scaleX, scaleY);

            if (matriz[i][j].esPuntoPoder) {
                casillaSprite.setColor(sf::Color(255, 215, 0, 200));
            }

            gVentana->draw(casillaSprite);

            // Pieza
            if (matriz[i][j].pieza) {
                Pieza* p = matriz[i][j].pieza;

                static std::map<std::string, sf::Texture> texturas;
                static bool texCargadas = false;
                if (!texCargadas) {
                    texturas["Caballero"].loadFromFile("assets/caballero.png");
                    texturas["Caballero_oscuro"].loadFromFile("assets/principe oscuro.png");
                    texturas["Golem"].loadFromFile("assets/golem.png");
                    texturas["PEKKA"].loadFromFile("assets/pekka.png");
                    texturas["Dragon"].loadFromFile("assets/dragon.png");
                    texturas["Dragon_infernal"].loadFromFile("assets/dragon infernal.png");
                    texturas["Arqueras"].loadFromFile("assets/arquera.png");
                    texturas["Reina_arquera"].loadFromFile("assets/reina arquera.png");
                    texturas["Valkiria"].loadFromFile("assets/valkiria.png");
                    texturas["Bandida"].loadFromFile("assets/bandida.png");
                    texturas["Curandera"].loadFromFile("assets/curandera.png");
                    texturas["Murcielago"].loadFromFile("assets/fenix.png");
                    texturas["Esbirro"].loadFromFile("assets/esbirro.png");
                    texturas["Dragon_electrico"].loadFromFile("assets/dragon electrico.png");
                    texturas["Mago"].loadFromFile("assets/mago.png");
                    texturas["Bruja"].loadFromFile("assets/bruja.png");
                    texCargadas = true;
                }

                std::string nombre = p->getNombre();
                if (texturas.count(nombre) > 0) {
                    sf::Sprite spr;
                    spr.setTexture(texturas[nombre]);
                    float escala = (TAM_CASILLA - 4.f) / texturas[nombre].getSize().x;
                    spr.setScale(escala, escala);
                    spr.setPosition(
                        OFFSET_X + j * TAM_CASILLA + 2.f,
                        OFFSET_Y + i * TAM_CASILLA + 2.f
                    );
                    window.draw(spr);
                }

                // Barra de vida
                float ratio = static_cast<float>(p->getVida()) / p->getVidaMax();
                sf::RectangleShape fv({ TAM_CASILLA - 10.f, 5.f });
                fv.setFillColor(sf::Color(40, 40, 40));
                fv.setPosition(OFFSET_X + j * TAM_CASILLA + 5.f, OFFSET_Y + i * TAM_CASILLA + TAM_CASILLA - 9.f);
                window.draw(fv);

                sf::RectangleShape bv({ (TAM_CASILLA - 10.f) * ratio, 5.f });
                bv.setFillColor(ratio > 0.5f ? sf::Color::Green : sf::Color::Red);
                bv.setPosition(fv.getPosition());
                window.draw(bv);
            }
        }
    }

    // Indicador de turno lateral
    sf::RectangleShape ind({ 18.f, TAM_CASILLA * TAM });
    ind.setPosition(OFFSET_X - 26.f, OFFSET_Y);
    ind.setFillColor(turnoActual == Equipo::Azul
        ? sf::Color(70, 130, 220, 200)
        : sf::Color(220, 60, 60, 200));
    window.draw(ind);
}

// =========================================================
// LÓGICA
// =========================================================
bool Tablero::esMovimientoLegal(Pieza* p, int xO, int yO, int xD, int yD) const {
    Pieza* dest = matriz[xD][yD].pieza;
    if (dest && dest->getEquipo() == p->getEquipo()) return false;

    int dist = std::abs(xD - xO) + std::abs(yD - yO);
    if (dist > p->getRangoTablero()) return false;

    if (p->getTipoMovimiento() == TipoMovimiento::Tierra) {
        if (xO != xD && yO != yD) return false;
        if (yO == yD) {
            int paso = (xD > xO) ? 1 : -1;
            for (int i = xO + paso; i != xD; i += paso)
                if (matriz[i][yO].pieza) return false;
        }
        else {
            int paso = (yD > yO) ? 1 : -1;
            for (int j = yO + paso; j != yD; j += paso)
                if (matriz[xO][j].pieza) return false;
        }
    }
    return true;
}

bool Tablero::moverPieza(int xO, int yO, int xD, int yD) {
    if (xO < 0 || xO >= TAM || yO < 0 || yO >= TAM || xD < 0 || xD >= TAM || yD < 0 || yD >= TAM) return false;
    Pieza* p = matriz[xO][yO].pieza;
    if (!p || p->getEquipo() != turnoActual) return false;

    if (esMovimientoLegal(p, xO, yO, xD, yD)) {
        Pieza* dest = matriz[xD][yD].pieza;
        if (dest && dest->getEquipo() != p->getEquipo()) {
            atacanteX = xO; atacanteY = yO;
            defensorX = xD; defensorY = yD;
            return true;
        }
        matriz[xD][yD].pieza = p;
        matriz[xO][yO].pieza = nullptr;
        finalizarTurno();
        return true;
    }
    return false;
}

void Tablero::aplicarCuracion() {
    for (int i = 0; i < TAM; ++i)
        for (int j = 0; j < TAM; ++j)
            if (matriz[i][j].pieza && matriz[i][j].esPuntoPoder)
                matriz[i][j].pieza->curar(15);
}

void Tablero::finalizarTurno() {
    aplicarCuracion();
    turnoActual = (turnoActual == Equipo::Azul) ? Equipo::Rojo : Equipo::Azul;
    contadorTurnos++;
}

// Cuenta cuántas piezas vivas tiene un equipo
int Tablero::contarPiezas(Equipo equipo) const {
    int count = 0;
    for (int i = 0; i < TAM; ++i)
        for (int j = 0; j < TAM; ++j)
            if (matriz[i][j].pieza && matriz[i][j].pieza->getEquipo() == equipo)
                count++;
    return count;
}

// Cuenta cuántos puntos de poder ocupa un equipo
int Tablero::contarPuntosPoder(Equipo equipo) const {
    int count = 0;
    for (int i = 0; i < TAM; ++i)
        for (int j = 0; j < TAM; ++j)
            if (matriz[i][j].esPuntoPoder &&
                matriz[i][j].pieza &&
                matriz[i][j].pieza->getEquipo() == equipo)
                count++;
    return count;
}

// Condición 3: el rival solo tiene una pieza y está encarcelada
bool Tablero::soloQuedaEncarcelada(Equipo equipo) const {
    int total = 0;
    int encarceladas = 0;
    for (int i = 0; i < TAM; ++i)
        for (int j = 0; j < TAM; ++j)
            if (matriz[i][j].pieza && matriz[i][j].pieza->getEquipo() == equipo) {
                total++;
                if (matriz[i][j].pieza->estaEncarcelada())
                    encarceladas++;
            }
    return (total == 1 && encarceladas == 1);
}

ResultadoVictoria Tablero::comprobarVictoria() const {
    //return ResultadoVictoria::GanaAzul; // TEST - borrar después
    // Condición 1: controlar los 5 puntos de poder
    if (contarPuntosPoder(Equipo::Azul) == 5)  return ResultadoVictoria::GanaAzul;
    if (contarPuntosPoder(Equipo::Rojo) == 5)  return ResultadoVictoria::GanaRojo;

    // Condición 2: el rival no tiene piezas
    if (contarPiezas(Equipo::Rojo) == 0)       return ResultadoVictoria::GanaAzul;
    if (contarPiezas(Equipo::Azul) == 0)       return ResultadoVictoria::GanaRojo;

    // Condición 3: al rival solo le queda una pieza encarcelada
    if (soloQuedaEncarcelada(Equipo::Rojo))    return ResultadoVictoria::GanaAzul;
    if (soloQuedaEncarcelada(Equipo::Azul))    return ResultadoVictoria::GanaRojo;

    return ResultadoVictoria::Ninguno;
}

Pieza* Tablero::getAtacante() const {
    return (atacanteX != -1) ? matriz[atacanteX][atacanteY].pieza : nullptr;
}

Pieza* Tablero::getDefensor() const {
    return (defensorX != -1) ? matriz[defensorX][defensorY].pieza : nullptr;
}

void Tablero::resolverCombate(Pieza* perdedor) {
    if (!perdedor) return;

    // Encontrar dónde está el perdedor
    int px = -1, py = -1;
    for (int i = 0; i < TAM && px == -1; ++i)
        for (int j = 0; j < TAM && px == -1; ++j)
            if (matriz[i][j].pieza == perdedor) { px = i; py = j; }

    if (px == -1) return;

    // El ganador ocupa la casilla del perdedor
    bool perdedorEsDefensor = (px == defensorX && py == defensorY);
    if (perdedorEsDefensor && atacanteX != -1) {
        delete matriz[defensorX][defensorY].pieza;
        matriz[defensorX][defensorY].pieza = matriz[atacanteX][atacanteY].pieza;
        matriz[atacanteX][atacanteY].pieza = nullptr;
    }
    else {
        delete matriz[px][py].pieza;
        matriz[px][py].pieza = nullptr;
    }

    atacanteX = atacanteY = defensorX = defensorY = -1;
}

bool Tablero::hayCombatePendiente() const {
    return (atacanteX != -1 && atacanteY != -1);
}

void Tablero::mueve(double dt) {}
void Tablero::tecla(unsigned char key) {}

// Busca al lanzador de hechizos del equipo dado y devuelve su posición
bool Tablero::buscarLanzador(Equipo equipo, int& fx, int& fy) const {
    for (int i = 0; i < TAM; ++i)
        for (int j = 0; j < TAM; ++j) {
            Pieza* p = matriz[i][j].pieza;
            if (!p || p->getEquipo() != equipo) continue;
            // Es LanzadorHechizos si su rangoTablero es 99 
            if (p->getRangoTablero() == 99) { fx = i; fy = j; return true; }
        }
    return false;
}

// Comprueba que en (x,y) hay una pieza del equipo dado
bool Tablero::buscarPieza(Equipo equipo, int x, int y) const {
    if (x < 0 || x >= TAM || y < 0 || y >= TAM) return false;
    return matriz[x][y].pieza && matriz[x][y].pieza->getEquipo() == equipo;
}

// Comprueba que (x,y) está vacía y dentro del tablero
bool Tablero::casillaLibre(int x, int y) const {
    if (x < 0 || x >= TAM || y < 0 || y >= TAM) return false;
    return matriz[x][y].pieza == nullptr;
}

bool Tablero::ejecutarHechizo(int idHechizo, int x, int y, int x2, int y2) {
    // Solo puede hechizar el equipo cuyo turno es
    Equipo equipoActual = turnoActual;
    Equipo equipoRival = (equipoActual == Equipo::Azul) ? Equipo::Rojo : Equipo::Azul;

    // Buscar al lanzador del equipo actual
    int lx = -1, ly = -1;
    if (!buscarLanzador(equipoActual, lx, ly)) return false;

    // Verificar que el lanzador puede usar este hechizo
    LanzadorHechizos* lanzador = dynamic_cast<LanzadorHechizos*>(matriz[lx][ly].pieza);
    if (!lanzador) return false;

    IdHechizo id = static_cast<IdHechizo>(idHechizo);
    if (!lanzador->puedeHechizar(id)) return false;

    bool exito = false;

    switch (id) {

        // ── CURAR: cura completamente a una pieza aliada en (x,y) ─────────────
    case IdHechizo::Curar:
        if (buscarPieza(equipoActual, x, y)) {
            matriz[x][y].pieza->curarTotal();
            exito = true;
        }
        break;

        // ── TELEPORTAR: mueve pieza aliada de (x,y) a (x2,y2) ────────────────
    case IdHechizo::Teleportar:
        if (buscarPieza(equipoActual, x, y) && casillaLibre(x2, y2)) {
            matriz[x2][y2].pieza = matriz[x][y].pieza;
            matriz[x][y].pieza = nullptr;
            exito = true;
        }
        break;

        // ── ENCARCELAR: inmoviliza una pieza rival en (x,y) ──────────────────
    case IdHechizo::Encarcelar:
        if (buscarPieza(equipoRival, x, y) &&
            !matriz[x][y].esPuntoPoder)  // los puntos de poder son inmunes
        {
            matriz[x][y].pieza->setEncarcelada(true);
            exito = true;
        }
        break;

        // ── REVIVIR: resucita la última pieza eliminada junto al lanzador ─────
    case IdHechizo::Revivir: {
        std::vector<Pieza*>& eliminadas =
            (equipoActual == Equipo::Azul) ? piezasEliminadasAzul : piezasEliminadasRojo;

        if (eliminadas.empty()) break;

        // Buscar casilla libre adyacente al lanzador
        int dx[] = { 0, 0, 1, -1, 1, -1, 1, -1 };
        int dy[] = { 1, -1, 0,  0, 1,  1,-1, -1 };
        int destX = -1, destY = -1;
        for (int d = 0; d < 8; ++d) {
            int nx = lx + dx[d], ny = ly + dy[d];
            if (casillaLibre(nx, ny)) { destX = nx; destY = ny; break; }
        }
        if (destX == -1) break; // no hay hueco

        Pieza* revivida = eliminadas.back();
        eliminadas.pop_back();
        revivida->curarTotal();
        matriz[destX][destY].pieza = revivida;
        exito = true;
        break;
    }

    default:
        break;
    }

    if (exito) {
        lanzador->marcarUsado(id);
        finalizarTurno();
    }
    return exito;
}