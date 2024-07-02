#include "pch.h"
#include "Testing_Utility_Functions.hpp"

sf::Font Rendering::Renderer::font;


void MockFixture::StealInstance() {
    if (Instance_Stolen) {
        return;
    }
    Instance_Stolen = true;
    TileManager* ptr = this;
    TileManager* singleton = &TileManager::instance();
    memcpy(theft, singleton, sizeof(TileManager));
    memcpy(singleton, ptr, sizeof(TileManager));

}

void MockFixture::returnInstance() {
    if (!Instance_Stolen) return;
    Instance_Stolen = false;
    TileManager* singleton = &TileManager::instance();
    memcpy(singleton, theft, sizeof(TileManager));
}

MockFixture::MockFixture() {
Instance_Stolen = false;
data = new char[(sizeof(TileManager))];
theft = reinterpret_cast<TileManager*>(data);
}

MockFixture::~MockFixture() {
    returnInstance();
    delete(data);
}

Graph::Geometry_Graph generate_Graph_in_Boundary(const int& minimum, const int& maximum, const int& max_nodes, const std::unordered_set<Geometry::grid_coordinate>& occupied, const Geometry::Coordinate_Transform& T, const double& edge_density) {

    std::random_device rd;
    std::mt19937 gen(rd());
    Geometry::grid_coordinate deltas[] = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };

    std::uniform_int_distribution<int> ran(0, 5);
    auto inv = T.inverse();

    std::uniform_int_distribution<int> x_dis(minimum, maximum);
    std::uniform_int_distribution<int> y_dis(minimum, maximum);

    std::vector<Graph::Collision_Leaf> output;
    std::unordered_map<Geometry::grid_coordinate, std::pair<int, int>> available;
    std::unordered_map<Geometry::grid_coordinate, int> M;

    auto inRegion = [&](const Geometry::grid_coordinate& point) {
        return point.i >= minimum && point.j >= minimum && point.i <= maximum && point.j <= maximum;
    };
    auto unoccupied = [&](const Geometry::grid_coordinate& point) {return occupied.find(T(point)) == occupied.end(); };
    auto nonCounted = [&](const Geometry::grid_coordinate& point) {return available.find(point) == available.end(); };
    auto notInside = [&](const Geometry::grid_coordinate& point) {return M.find(point) == M.end(); };

    //int trace = T.x_mat.i + T.y_mat.j;
    //int rotationAdjustment;
    //if (trace > 0) {
    //    rotationAdjustment = T.x_mat.i + 5 * T.y_mat.j;
    //}
    //else {
    //    rotationAdjustment = 3 + T.y_mat.j - T.x_mat.i;
    //}





    for (int i = 0; i < 1000; ++i) {
        int x_start = x_dis(gen);
        int y_start = y_dis(gen);
        if (occupied.find({ x_start,y_start }) == occupied.end()) {
            output.push_back({});
            Geometry::grid_coordinate start = { x_start, y_start };
            output[0].coord = inv(start);
            output[0].collision.center_ = inv(start);
            output[0].collision.radius_ = ATOMIC_RADIUS;
            M[inv(start)] = 0;
            for (int j = 0; j < 6; ++j) {
                auto other = (inv(start) + deltas[j]);
                if (inRegion(T(other)) && unoccupied((other))) {
                    available[other] = { 0,j };
                }
            }
            break;
        }
    }

    while (output.size() < max_nodes && !available.empty()) {
        auto front = *available.begin();
        for (int j = 0; j < 6; ++j) {
            auto new_point = deltas[j] + front.first;
            if (inRegion(T(new_point)) && unoccupied(new_point) && nonCounted(new_point) && notInside(new_point)) {
                available[new_point] = { output.size(), j };
            }
        }
        M[front.first] = output.size();
        output.push_back({});
        output.back().coord = front.first;
        output.back().collision.center_ = front.first;
        output.back().collision.radius_ = ATOMIC_RADIUS;
        output.back().children[1 + (3 + front.second.second) % 6] = front.second.first;
        output[front.second.first].children[front.second.second + 1] = output.size() - 1;
        available.erase(front.first);
        
    }


    std::uniform_real_distribution<double> edge_dist(0, 1);
    for (int i = 0; i < output.size(); ++i) {
        for (int j = 1; j < 7; ++j) {
            if (output[i].children[j] != -1) {
                continue;
            }
            if (M.find(output[i].coord + deltas[j - 1]) != M.end()) {
                if (edge_dist(gen) < edge_density) {
                    output[i].children[j] = M[output[i].coord + deltas[j - 1]];
                    output[M[output[i].coord + deltas[j - 1]]].children[1 + (2 + j) % 6] = i;
                }

            }
        }
    }

    return { {M},output };




}


std::vector<Graph::Collision_Leaf> UTILITY::generate_nodes(int n) {
    std::unordered_map<Geometry::grid_coordinate, int> M;
    Geometry::grid_coordinate current = { 0,0 };
    M.insert({ current,0 });
    std::random_device rd;
    std::mt19937 gen(rd());
    Geometry::grid_coordinate deltas[] = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };
    int inverse[] = { 3,4,5,0,1,2 };
    std::uniform_int_distribution<int> ran(0, 5);

    std::vector<Graph::Collision_Leaf> output;
    output.push_back({});
    output[0].coord = { 0,0 };
    output[0].children[0] = 0;

    while (M.size() < n) {
        int direction = ran(gen);
        current = current + deltas[direction];
        int size = M.size();
        if (M.find(current) == M.end()) {
            output.push_back({});
            output.back().coord = current;
            output.back().children[0] = 0;
            int prev = M[current - deltas[direction]];
            output[prev].children[1 + direction] = M.size();
            M[current] = size;
            output.back().children[1 + inverse[direction]] = prev;

        }
    }
    return output;
}

TileManager::TESTING_ACCESSOR MockFixture::ACCESSOR;

std::vector<int>& TileManager::TESTING_ACCESSOR::getconnectivityMap(TileManager& T) {
    return T.connectivityMap;
}

std::vector<std::vector<int>>& TileManager::TESTING_ACCESSOR::getpartitionSizes(TileManager& T) {
    return T.partitionSizes;
}

std::vector<std::vector<std::pair<int, int>>>& TileManager::TESTING_ACCESSOR::getpartitions(TileManager& T) {
    return T.partitions;
}

std::vector<std::vector<int>> TileManager::TESTING_ACCESSOR::gettasks(TileManager& T) {
    return T.tasks;
}

std::vector<int>& TileManager::TESTING_ACCESSOR::getmoleculeOwners(TileManager& T) {
    return T.moleculeOwners;
}

std::vector<int>& TileManager::TESTING_ACCESSOR::getpartitionLinks(TileManager& T) {
    return T.partitionLinks;
}

std::vector<int>& TileManager::TESTING_ACCESSOR::getTrivialTasks(TileManager& T) {
    return T.TrivialTasks;
}