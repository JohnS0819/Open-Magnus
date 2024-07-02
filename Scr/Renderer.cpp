#include "Renderer.hpp"
#include "Tile.hpp"
#include <memory>
#include "Molecule.hpp"
#include "Mechanism.hpp"
#include "Connections.hpp"

Graph::Geometry_Graph generateNodes_(int n) {
    std::unordered_map<Geometry::grid_coordinate, int> M;
    Geometry::grid_coordinate current = { 0,0 };
    M.insert({ current,0 });
    std::random_device rd;
    std::mt19937 gen(rd());
    Geometry::grid_coordinate deltas[] = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };
    int inverse[] = { 3,4,5,0,1,2 };
    std::uniform_int_distribution<int> ran(0, 5);

    std::vector<Graph::Collision_Leaf> output;

    auto transform = [](const Geometry::grid_coordinate& point) { return Geometry::coordinate(point.i  + 0.5f * point.j, 0.866025403784f * point.j); };
    output.push_back({});
    output[0].coord = { 0,0 };
    output[0].children[0] = -1;
    output[0].collision.center_ = { 0,0 };
    output[0].collision.radius_ = ATOMIC_RADIUS;
    while (M.size() < n) {
        int direction = ran(gen);
        current = current + deltas[direction];
        int size = M.size();
        if (M.find(current) == M.end()) {
            output.push_back({});
            output.back().coord = current;
            output.back().collision.center_ = transform(current);
            output.back().collision.radius_ = ATOMIC_RADIUS;
            output.back().children[0] = -1;
            int prev = M[current - deltas[direction]];
            output[prev].children[1 + direction] = M.size();
            M[current] = size;
            output.back().children[1 + inverse[direction]] = prev;

        }
    }
    Graph::Geometry_Graph G;
    G.nodes = output;
    G.coord_map.push_back(M);

    return G;
}

Rendering::Molecule_Primitive generate_nodes(int n) {
    std::unordered_map<Geometry::grid_coordinate, int> M;
    Geometry::grid_coordinate current = { 0,0 };
    M.insert({ current,0 });
    std::random_device rd;
    std::mt19937 gen(rd());
    std::complex<int> deltas[] = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };
    std::uniform_int_distribution<int> ran(0, 5);
    while (M.size() < n) {
        int direction = ran(gen);
        current.i += deltas[direction].real();
        current.j += deltas[direction].imag();
        int size = M.size();
        if (M.find(current) == M.end()) {
            M[current] = size;
        }
    }
    Rendering::Molecule_Primitive output;

    for (auto i : M) {
        output.atoms.push_back(Rendering::Atom_Primitive({ i.first.i,i.first.j }));
    }
    return output;


}



template<>
void Rendering::Render_Window::draw(const Graph::Collision_Hierarchy& graph) {
    auto& nodes = graph.getNodes();
    std::vector<sf::Color> colors({sf::Color::Green,sf::Color::Red,sf::Color::Blue,sf::Color::Magenta,sf::Color::Cyan,sf::Color::White,sf::Color::Yellow});
    for (int j = 0; j < nodes.size(); ++j) {
        sf::Color color = colors[j % colors.size()];
        for (auto& i : nodes[j]) {
            sf::CircleShape circle(128 * i.collision.radius_);
            circle.setFillColor(sf::Color::Transparent);
            circle.setOutlineColor(color);
            circle.setOutlineThickness(8 * j + 4); // Adjust outline thickness as needed
            circle.setPosition(128 * (i.collision.center_.x - i.collision.radius_), 128 * (i.collision.center_.y - i.collision.radius_));
            RenderWindow::draw(circle);
        }

    }

}



Rendering::Renderer::Renderer() : window(sf::VideoMode(1600, 900), "SFML Graph Rendering"){

    view = new Rendering::View;
    view->setWindow(window);
}

const Rendering::View* Rendering::Renderer::getView() const {
    return view;
}

const Rendering::Render_Window& Rendering::Renderer::getWindow() const {
    return window;
}


Rendering::Renderer& Rendering::Renderer::instance() {
    static Renderer singleton_;
    return singleton_;
}

sf::Font Rendering::Renderer::font;

void Rendering::Renderer::Render() {
    using namespace Rendering;

    if (!font.loadFromFile("times.ttf")) {
        std::cerr << "NOOOOO" << std::endl;
    }
    int X_DIM = 1182;
    int Y_DIM = 1024;

    sf::Texture texture;
    if (!texture.loadFromFile("Complete_Lattice.png")) {
        std::cerr << "NOOOOO" << std::endl;
    }
    sf::Text text;

    text.setFont(font);
    text.setFillColor(sf::Color::White);
    text.setScale(1, -1);
    text.setOrigin(0, 25);
    // Define colors for nodes and connections
    sf::Color nodeColor = sf::Color::Blue;
    sf::Color connectionColor = sf::Color::Red;
    //Graph::Collision_Hierarchy graph(generate_nodes(5));

    GridTile TESTER;
    TESTER.spots.resize(2);
    TESTER.x = 0;
    TESTER.y = 0;
    TESTER.spots[0].location = { -1,0 };
    TESTER.spots[1].location = { -2,0 };
    TESTER.type = GridTile::Type::disconnector;

    GridTile TESTER2;
    TESTER2.spots.resize(2);
    Geometry::grid_coordinate uncastedCoord = { 1,-2 };
    Geometry::coordinate castedCoord = uncastedCoord;
    TESTER2.x = castedCoord.x;
    TESTER2.y = castedCoord.y;
    TESTER2.spots[0].location = { 1,-2 };
    TESTER2.spots[1].location = { 2,-2 };
    TESTER2.type = GridTile::Type::connecter;

    TESTER.clear();
    TileManager::instance().insert(TESTER);
    TileManager::instance().insert(TESTER2);
    TileManager::instance().build();
    
    Geometry::grid_coordinate A(0, 0);
    Atom_Primitive P(A);

    A.i = 1;
    A.j = 1;
    Atom_Primitive P2(A);

    A.i = 0;
    A.j = 4;
    Atom_Primitive P1(A);

    A.i = 4;
    A.j = 0;

    Atom_Primitive P3(A);
    // Variables to handle panning
    bool isPanning = false;
    sf::Vector2f panStart;
    // Main render loop
    float R = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();
    Motion::Translation rotation;
    rotation.delta_ = { 1,0 };
    Motion_Animation<Motion::Translation> Anim(rotation);
    Time_Singleton::instance().setAnimationDuration(std::chrono::duration<double>(0.02));
    Time_Singleton::instance().nextFrame();


    auto PREVSIZE = view->getSize();

    std::vector<Molecule> MOLEPOINTERS;

    /*Graph::Collision_Hierarchy sanity(generateNodes_(2000));
    while (sanity.getNodes().back().size() > 1) {
        sanity.subdivide();
    }*/
    /*for (auto& i : sanity.getNodes()[0]) {
        std::cerr << i.coord << std::endl;
    }*/

   


    Graph::Geometry_Graph straightLine;
    straightLine.coord_map.resize(1);
    for (int i = 0; i < 20; ++i) {
        straightLine.nodes.push_back({});
        auto& current = straightLine.nodes.back();
        current.coord.i = -i - 2;
        current.coord.j = 0;
        current.collision.center_ = current.coord;
        current.collision.radius_ = ATOMIC_RADIUS;
        if (i != 19) {
            current.children[5] = i + 1;
        }
        if (i != 0) {
            current.children[2] = i - 1;
        }
        straightLine.coord_map[0][current.coord] = i;
    }
    Graph::Collision_Hierarchy sanity(straightLine);

    while (sanity.getNodes().back().size() > 1) {
        sanity.subdivide();
    }

    Molecule MOLE= std::move(sanity);
    MoleculeManager::instance().insert(&MOLE);

    auto RENDERTEST = MoleculeManager::instance().generateRenderables();
    Geometry::grid_coordinate SOURCE_;
    SOURCE_.i = -2;
    SOURCE_.j = 2;

   /* Instructions::Instruction<Instructions::Type::Rotate_CCW> I;
    head_.queryable_ = true;
    const Instructions::GenericInstruction* ptr = &I;
    head_.ptr_ = &ptr;
    Geometry::grid_coordinate SOURCE_;
    SOURCE_.i = -10;
    SOURCE_.j = 10;
    I.setDataSource(&SOURCE_);
    I.updateMotion();*/


    std::vector<std::shared_ptr<Instructions::GenericInstruction>> _tape_;
   // _tape_.push_back(std::make_shared<Instructions::Instruction<Instructions::Type::Rotate_CCW>>(Instructions::Instruction<Instructions::Type::Rotate_CCW>()));
    
    Geometry::grid_coordinate translationMotion;
    translationMotion.i = 1;
    translationMotion.j = 0;


    Geometry::grid_coordinate translationMotion2;
    translationMotion2.i = -1;
    translationMotion2.j = 0;
    



    Mechanism mechanism;
    
    
    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Grab>>(new Instructions::Instruction<Instructions::Type::Grab>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Translate_Pos>>(new Instructions::Instruction<Instructions::Type::Translate_Pos>));
    _tape_.back()->setDataSource(&translationMotion);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Rotate_CCW>>(new Instructions::Instruction<Instructions::Type::Rotate_CCW>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();
    
    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Translate_Pos>>(new Instructions::Instruction<Instructions::Type::Translate_Pos>));
    _tape_.back()->setDataSource(&translationMotion);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Release>>(new Instructions::Instruction<Instructions::Type::Release>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Translate_Pos>>(new Instructions::Instruction<Instructions::Type::Translate_Pos>));
    _tape_.back()->setDataSource(&translationMotion2);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Translate_Pos>>(new Instructions::Instruction<Instructions::Type::Translate_Pos>));
    _tape_.back()->setDataSource(&translationMotion2);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Rotate_CW>>(new Instructions::Instruction<Instructions::Type::Rotate_CW>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    /*_tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Rotate_CCW>>(new Instructions::Instruction<Instructions::Type::Rotate_CCW>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Rotate_CCW>>(new Instructions::Instruction<Instructions::Type::Rotate_CCW>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Release>>(new Instructions::Instruction<Instructions::Type::Release>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Rotate_CW>>(new Instructions::Instruction<Instructions::Type::Rotate_CW>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Grab>>(new Instructions::Instruction<Instructions::Type::Grab>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();

    _tape_.push_back(std::shared_ptr<Instructions::Instruction<Instructions::Type::Rotate_CW>>(new Instructions::Instruction<Instructions::Type::Rotate_CW>));
    _tape_.back()->setDataSource(&mechanism);
    _tape_.back()->updateMotion();*/

    
    //Mechanism mechanism = Mechanism(std::move(_tape_));
    
    mechanism.tape.Instructions_ = std::move(_tape_);

    Mechanism::Head head_;
    head_.i = -2;
    head_.j = 0;
    
    head_.queryable_ = true;
    //const Instructions::GenericInstruction* ptr = &I;
    
    mechanism.heads_.push_back(&head_);
    
    head_.ptr_ = &mechanism.currentInstruction;
    mechanism.read();


    //mechanism.heads_.push_back(&head_);
    mechanism.intersect(MOLE);

    /*if (!MOLE.setMechanism(mechanism)) {
        std::cerr << "WTF" << std::endl;
        return;
    }*/
    window.setFramerateLimit(120);


    double ANGLE = 0;
    bool converted = false;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle zooming with the scroll wheel
            view->processEvent(event);
        }

        view->update();
        sf::Color Neutral;
        Neutral.r = 0;
        Neutral.b = 0;
        Neutral.g = 0;
        Neutral.a = 0;
        window.setView(*view);
        window.clear(Neutral);
        //window.draw(graph);
        //window.draw(&P.vertices[0],72,sf::Triangles);
        //window.draw(&P1.vertices[0], 72, sf::Triangles);
        //window.draw(&P3.vertices[0], 72, sf::Triangles);
        sf::RenderStates state;
        /*sf::Transform T;
        T.translate({ 1000.0f, 0.0f });
        T.rotate(R);
        T.translate({ -1000.0f,0.0f });*/
        auto now = std::chrono::high_resolution_clock::now();
        sf::Transform T = Anim.getTransform();
        //std::cerr << '(' << v32.x << '\t' << v32.y << ')' << std::endl;
        state.transform = T;
        //window.draw(&Molecule.atoms[0].vertices[0], Molecule.atoms.size() * 3 * NUMBER_OF_CIRCLE_POINTS, sf::Triangles,state);
        //window.draw(grid);
       /* for (int p = 0; p < spots.size(); ++p) {
            window.draw(spots[p]);
        }*/
        
       
        Geometry::Grid::instance().UpdateGridScaling( window);
        window.draw(Geometry::Grid::instance());
        //window.draw(sanity);

        
        text.setPosition(window.mapPixelToCoords(sf::Vector2i(0, 500)));
        auto SIZE = view->getSize();
        auto pos = view->getCenter();
        text.setScale(SIZE.x / PREVSIZE.x, -SIZE.y / PREVSIZE.y);
        text.setString(std::to_string(pos.x) + "," + std::to_string(pos.y));


        window.RenderWindow::draw(text);
        for (auto& moles : RENDERTEST) {
            window.draw(moles);
        }
        double constant = Time_Singleton::instance().getInterpolation();
        ANGLE += 0.005;
        /*if (ANGLE < 1.0471975512) {
            for (auto& moles : RENDERTEST) {
                moles.Orientation.Rotation.x = cos(ANGLE);
                moles.Orientation.Rotation.y = sin(ANGLE);
            }
        }*/
        window.draw(mechanism);
        if (constant < 1) {
            for (auto& moles : RENDERTEST) {
                moles.SFTRANSFORM = moles.ptr->getRenderTransform();
               /* sf::Transform TT;
                TT.rotate(60 * constant, { -256 ,0 });
                moles.SFTRANSFORM = TT;*/
            }
        }
        else  {
            converted = true;
            auto& temptemp = MoleculeManager::instance().TESTUTIL();
            for (auto& n : temptemp) {
                n->updateOrientation();
            }
            //MOLE.updateOrientation();
           /* TileManager::instance().reset();
            TileManager::instance().query(MOLE, 0);
            MOLE.callbackConnections(0);
            TileManager::instance().generateConnectivityMap();
            TileManager::instance().generatePartitions();
            TileManager::instance().generateTasks();
            TileManager::instance().configureTrivialTask(0);
            ConnectionManager::instance().completeTrivial(MOLE, 0);*/
            MoleculeManager::instance().TESTING();
            RENDERTEST = MoleculeManager::instance().generateRenderables();
            for (auto& moles : RENDERTEST) {
                moles.SFTRANSFORM = moles.ptr->getRenderTransform();
            }
            //head_.updatePosition();
            mechanism.read();
            auto& blabla = MoleculeManager::instance().TESTUTIL();
            for (auto& n : blabla) {
                mechanism.intersect(*n);
            }
            //mechanism.intersect(MOLE);
            Time_Singleton::instance().nextFrame();
        }
        //else {
        //    for (auto& moles : RENDERTEST) {
        //        moles.SFTRANSFORM = MOLE.getRenderTransform();
        //        /* sf::Transform TT;
        //         TT.rotate(60 * constant, { -256 ,0 });
        //         moles.SFTRANSFORM = TT;*/
        //    }
        //}
        window.draw(TESTER);
        window.draw(TESTER2);


       /* window.RenderWindow::draw(C1);
        window.RenderWindow::draw(C2);
        window.RenderWindow::draw(C3);
        window.RenderWindow::draw(C4);*/
        //window.draw(&P2.vertices[0], 72, sf::Triangles,T);
        window.display();
        Time_Singleton::instance().updateTime();
    }


}