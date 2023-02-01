#include "Player.hh"
#include "queue"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Mir

// DISCLAIMER: The following Demo player is *not* meant to do anything
// sensible. It is provided just to illustrate how to use the API.
// Please use AINull.cc as a template for your player.

struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }


  /**
   * Types and attributes for your player can be defined here.
   */
  const vector<Dir> dirs = {Up,Down,Left,Right};

    //indica si els alive es del meu equip
    bool hayCompa(Pos p) {
        Cell c = cell(p);
        if (c.id != -1) {//no es zombie
            Unit u = unit(c.id);
            if (u.player == me()) return true;
        }
        return false;
    }

    bool hay_comida(Pos p) {
        Cell c = cell(p);
        if (c.food) return true;
        return false;
    }

    bool acces(Pos p) {
        Cell c = cell(p);
        //pendiente de evitar muerto
        if (c.type != Waste) return true;
        return false;
    }



    //mira si es pot accedir
/*   bool acces(Pos p) {
        Cell c = cell(p);
        Unit u = unit(c.id);
        //pendiente de evitar muerto
        if (c.type != Waste){
            if(((u.type) != Dead and c.id != -1) or c.id == -1) return true;//no sea ni zombi ni muerto
        }
        return false;
    }
*/
    //nos devolvera la posicion de un enemigo tanto zombi o uno del clan contrario
    bool hay_enemigo(Pos p) {
        Cell c = cell(p);
        // Si está ocupada...
        if (c.id != -1) {//entonces sera un zombi o una persona
            Unit u = unit(c.id);
            //mato a lo que no sea un muerto o a uno de mi equipos
            if (u.player != me() and u.type != Dead) return true;
        }
        return false;
    }

    bool hay_muerto(Pos p) {
        Cell c = cell(p);
        // Si está ocupada...
        if (c.id != -1) {//entonces sera un zombi o una persona
            Unit u = unit(c.id);
            //mato a lo que no sea un muerto o a uno de mi equipos
            if (u.type == Dead) return true;
        }
        return false;
    }

    bool hay_zombie_lado(Pos p) {
        Cell c = cell(p);
        // Si está ocupada...
        if (c.id != -1) {//entonces sera un zombi o una persona
            Unit u = unit(c.id);
            //mato a lo que no sea un muerto o a uno de mi equipos
            if (u.type == Zombie) return true;
        }
        return false;
    }


    bool hay_zombie(Pos p, Dir d){
        Pos p1(-1,-1),p2(-1,-1);
        if(d==Up) {
            p1 = p+UL;
            p2 = p+RU;
        }
        else if(d == Right){
            p1 = p+RU;
            p2 = p+DR;
        }
        else if(d==Left){
            p1 = p+UL;
            p2 = p+LD;
        }
        else {
            p1 = p+LD;
            p2 = p+DR;
        }

        if(pos_ok(p1) and acces(p1) and cell(p1).id != -1 and unit(cell(p1).id).type == Zombie) return true;
        if(pos_ok(p2) and acces(p2) and cell(p2).id != -1 and unit(cell(p2).id).player == -1) return true;
        return false;
    }


    Dir kill_bfs(Pos ori) {

        // Matriz dir ini None:
        vector<vector<Dir>> MDir(60, vector<Dir>(60, DR));
        // Cola pendientes de visi
        queue<Pos> pendents;
        // marcamos posicions nuestra para no volver
        MDir[ori.i][ori.j] = Up;

        // obtiene la pos adyacente válida donde mover
        for (int i = 0; i < 4; ++i) {
            Pos nou = ori + dirs[i];
            // Chequea si esta en el tablero
            if (pos_ok(nou) and acces(nou) and not hayCompa(nou)){
                pendents.push(nou);
                MDir[nou.i][nou.j] = dirs[i];
                // Si hay un enemigo devolvemos la dirección ya que querremos ir hacia él (atacarle)
                if (hay_enemigo(nou)) return dirs[i];
                if(hay_comida(nou)) return dirs[i];
            }
        }

        //si aun hay algo para mirar
        while (not pendents.empty()) {
            Pos actual = pendents.front();
            pendents.pop();
            // Visit adyacentes
            for (int j = 0; j < 4; ++j) {
                Pos nou = actual + dirs[j];
                // no vis, marcamos dir y la ponemos a pendents
                if (pos_ok(nou) and MDir[nou.i][nou.j] == DR){
                    // si hay player , devolvemos esa dirección:
                    if (acces(nou)) {
                        if (hay_enemigo(nou)) return MDir[actual.i][actual.j];
                        pendents.push(nou);
                        // Copy dir desde donde la visitamos
                        MDir[nou.i][nou.j] = MDir[actual.i][actual.j];
                    }
                }
            }
        }
        // none si no hay nada para acceder
        return DR;
    }

    Dir food_bfs(Pos ori) {

        // Matriz dir ini None:
        vector<vector<Dir>> MDir(60, vector<Dir>(60, DR));
        // Cola pendientes de visi
        queue<Pos> pendents;
        // marcamos posicions nuestra para no volver
        MDir[ori.i][ori.j] = Up;

        // obtiene la pos adyacente válida donde mover
        for (int i = 0; i < 4; ++i) {
            Pos nou = ori + dirs[i];
            // Chequea si esta en el tablero y evitamos zombie
            if (pos_ok(nou) and acces(nou) and not hayCompa(nou)) {// and not hay_muerto(nou)){
                if (not hay_zombie(ori,dirs[i])) pendents.push(nou);
                MDir[nou.i][nou.j] = dirs[i];
                // Si hay nemigo devolvemos dir para atacarlo
                if(hay_comida(nou)) return dirs[i];
                if (hay_enemigo(nou)) return dirs[i];
            }
        }

        //mientras tengamos para mirar
        while (not pendents.empty()) {
            Pos actual = pendents.front();
            pendents.pop();
            // Visitamos a las adyacentes:
            for (int j = 0; j < 4; ++j) {
                Pos nou = actual + dirs[j];
                // no vis, marcamos dir y la ponemos a pendents
                if (pos_ok(nou) and acces(nou) and MDir[nou.i][nou.j] == DR){
                    // si hay player , devolvemos esa dirección:
                        if (hay_comida(nou)) return MDir[actual.i][actual.j];
                        if (hay_zombie_lado(nou)) return MDir[actual.i][actual.j];
                        pendents.push(nou);
                        // Copy dir desde donde la visitamos
                        MDir[nou.i][nou.j] = MDir[actual.i][actual.j];

                }
            }
        }
        // none si no hay nada para acceder
        return DR;
    }


      //bulce que mou cadascun del meus persons
    void moveperson(vector<int>& alive){
        int Nalive = int(alive.size());
        for (int i = 0; i < Nalive; ++i) {
            //vamos a matar
            if(round() < 150) {
                move(alive[i],food_bfs(unit(alive[i]).pos));
                /*
                if(unit(i).type == Alive and unit(i).rounds_for_zombie == -1) move(alive[i],food_bfs(unit(alive[i]).pos));
                else move(alive[i],kill_bfs(unit(alive[i]).pos));
                */

            }
            else move(alive[i],kill_bfs(unit(alive[i]).pos));//infectat

        }
    }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {

    double st = status(me());
    vector<int> alive = alive_units(me());
    moveperson(alive);

  }
};

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
