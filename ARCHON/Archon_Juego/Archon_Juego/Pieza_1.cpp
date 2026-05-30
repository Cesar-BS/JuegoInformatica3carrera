#include "Pieza.h"
#include <algorithm>


// CONSTRUCTOR Y MÉTODOS DE LA CLASE BASE PIEZA


Pieza::Pieza(Equipo equipo, const std::string& nombre, TipoMovimiento tipoMov, TipoArma tipoArma,
    int vidaMax, int velocidad, int fuerzaAtaque, int velocidadAtaque, int alcanceAtaque, int rangoTablero)
    : equipo_(equipo)
    , nombre_(nombre)
    , tipoMov_(tipoMov)
    , tipoArma_(tipoArma)
    , vida_(vidaMax)
    , vidaMax_(vidaMax)
    , velocidad_(velocidad)
    , fuerzaAtaque_(fuerzaAtaque)
    , velocidadAtaque_(velocidadAtaque)
    , alcanceAtaque_(alcanceAtaque)
    , rangoTablero_(rangoTablero)
{
}

void Pieza::recibirDano(int cantidad) {
    vida_ = std::max(0, vida_ - cantidad);
}

void Pieza::curar(int cantidad) {
    vida_ = std::min(vidaMax_, vida_ + cantidad);
}


// PIEZAS DE TIERRA


Caballero::Caballero(Equipo equipo)
    : PiezaTierra(equipo, "Caballero", TipoMovimiento::Tierra, TipoArma::CuerpoACuerpo, 60, 130, 15, 400, 1, 3)
{
}

Caballero_oscuro::Caballero_oscuro(Equipo equipo)
    : PiezaTierra(equipo, "Caballero_oscuro", TipoMovimiento::Tierra, TipoArma::CuerpoACuerpo, 60, 130, 15, 400, 1, 3)
{
}

Golem::Golem(Equipo equipo)
    : PiezaTierra(equipo, "Golem", TipoMovimiento::Tierra, TipoArma::CuerpoACuerpo, 200, 55, 40, 1100, 1, 2)
{
}

PEKKA::PEKKA(Equipo equipo)
    : PiezaTierra(equipo, "PEKKA", TipoMovimiento::Tierra, TipoArma::CuerpoACuerpo, 200, 55, 40, 1100, 1, 2)
{
}

Dragon::Dragon(Equipo equipo)
    : PiezaTierra(equipo, "Dragon", TipoMovimiento::Tierra, TipoArma::Magia, 110, 95, 25, 700, 4, 3)
{
}

Dragon_infernal::Dragon_infernal(Equipo equipo)
    : PiezaTierra(equipo, "Dragon_infernal", TipoMovimiento::Tierra, TipoArma::Magia, 110, 95, 25, 700, 4, 3)
{
}

Arqueras::Arqueras(Equipo equipo)
    : PiezaTierra(equipo, "Arqueras", TipoMovimiento::Tierra, TipoArma::Proyectil, 70, 90, 20, 600, 7, 3)
{
}

Reina_arquera::Reina_arquera(Equipo equipo)
    : PiezaTierra(equipo, "Reina_arquera", TipoMovimiento::Tierra, TipoArma::Proyectil, 70, 90, 20, 600, 7, 3)
{
}


// PIEZAS VOLADORAS


Valkiria::Valkiria(Equipo equipo)
    : PiezaVuelo(equipo, "Valkiria", TipoMovimiento::Vuelo, TipoArma::CuerpoACuerpo, 90, 140, 22, 500, 2, 4)
{
}

Bandida::Bandida(Equipo equipo)
    : PiezaVuelo(equipo, "Bandida", TipoMovimiento::Vuelo, TipoArma::Magia, 90, 120, 18, 550, 3, 4)
{
}

Curandera::Curandera(Equipo equipo)
    : PiezaVuelo(equipo, "Curandera", TipoMovimiento::Vuelo, TipoArma::Magia, 80, 150, 28, 650, 5, 5)
{
}

Murcielago::Murcielago(Equipo equipo)
    : PiezaFuego(equipo, "Murcielago",
            TipoMovimiento::Vuelo, TipoArma::ExplosionArea,
            /*vida*/ 100,  /*vel*/ 130,
            /*fuerzaAtaque*/   35,  /*velocidadAtaque*/ 900,
            /*alcanceAtaque*/    3,  /*rangoTablero*/ 4)
{}



Esbirro::Esbirro(Equipo equipo)
    : PiezaFuego(equipo, "Esbirro",
        TipoMovimiento::Vuelo, TipoArma::ExplosionArea,
        /*vida*/ 100,  /*vel*/ 130,
        /*fuerzaAtaque*/   35,  /*velocidadAtaque*/ 900,
        /*alcanceAtaque*/    3,  /*rangoTablero*/ 4)
{}


void PiezaFuego::activarFormaFuego() {
    if (!formaFuego_) {
        formaFuego_ = true;
        tiempoFormaFuego_ = DURACION_FORMA_FUEGO;
    }
}

void PiezaFuego::actualizarArena(float deltams) {
    if (formaFuego_) {
        tiempoFormaFuego_ -= deltams;
        if (tiempoFormaFuego_ <= 0.0f) {
            formaFuego_ = false;
            tiempoFormaFuego_ = 0.0f;
        }
    }
}

Dragon_electrico::Dragon_electrico(Equipo equipo)
    : PiezaVuelo(equipo, "Dragon_electrico", TipoMovimiento::Vuelo, TipoArma::Proyectil, 150, 100, 38, 800, 8, 5)
{
}


// LANZADORES DE HECHIZOS


LanzadorHechizos::LanzadorHechizos(Equipo equipo, const std::string& nombre)
    : PiezaTeleport(equipo, nombre, TipoMovimiento::Teletransporte, TipoArma::Magia, 80, 110, 20, 750, 5, 99)
{
}

bool LanzadorHechizos::puedeHechizar(IdHechizo hechizo) const {
    return !hechizosUsados_[static_cast<int>(hechizo)];
}

void LanzadorHechizos::marcarUsado(IdHechizo hechizo) {
    hechizosUsados_[static_cast<int>(hechizo)] = true;
}

Mago::Mago(Equipo equipo) : LanzadorHechizos(equipo, "Mago") {}

Bruja::Bruja(Equipo equipo) : LanzadorHechizos(equipo, "Bruja") {}


// CHANGELING


Changeling::Changeling(Equipo equipo)
    : PiezaVuelo(equipo, "Changeling", TipoMovimiento::Vuelo, TipoArma::Magia, 100, 120, 25, 600, 4, 4)
{
}

void Changeling::adoptarForma(const Pieza& objetivo) {
    tipoAdoptado_ = objetivo.getTipoMovimiento();
    rangoAdoptado_ = objetivo.getRangoTablero();
    adoptado_ = true;
}

TipoMovimiento Changeling::getTipoMovimientoEfectivo() const {
    return adoptado_ ? tipoAdoptado_ : tipoMov_;
}

int Changeling::getRangoTableroEfectivo() const {
    return adoptado_ ? rangoAdoptado_ : rangoTablero_;
}