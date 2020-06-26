#include <algorithm>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>
#include <random>
#include <unistd.h>
#include <ctime>

int global_num = 0;
std::vector<std::vector<double>> graph;

struct Connection {
    int r_id_1;
    int line_1;
    int r_id_2;
    int line_2;
};

struct BW {
    int road_num = 0;
    std::vector<int> lane;
};

class Road;
class Junction;
class Car;

std::vector<Road> roads;
std::vector<Junction> juncs;
std::vector<std::vector<std::vector<BW>>> best_ways;

class Road {
    public:
        int id;
        int from;
        int to;
        int num_of_lines;
        int len;
        int max_speed;
        std::vector<std::vector<int>> free_pos;
        std::vector<Car> road_cars;
    
        Road(int i, int f, int t, int l, int ln, int ms) {
            id = i;
            from = f;
            to = t;
            num_of_lines = l;
            ln /= 2;
            len = ln;
            for (int i = 0; i < num_of_lines; ++i) {
                std::vector<int> tmp(ln, 1);
                free_pos.push_back(tmp);
            }
            max_speed = ms;
        }

        std::pair<int, int> get_ft() {
            return std::make_pair(from, to);
        }
};

class Junction {
    public:
        int j_id;
        int start;
        std::vector<Road> roads;
        std::vector<Connection> conns;
};

class Car {
    public:
        int id;
        int type;
        int size;
        int from;
        int to;
        int line;
        int speed;
        int accel;
        int road_block;
        int cur_road;
        int rest;
        std::vector<BW> way;

        Car (int ty, int f, int t) {
            id = global_num++;
            type = ty;
            from = f;
            to = t;
            line = 0;
            speed = 0;
            if (type == 0) {
                size = 2;
                accel = 8;
            } else {
                size = 5;
                accel = 4;
            }
            if (best_ways[f][t].empty()) {
                make_way(f);
            }
            road_block = size;
            way = best_ways[f][t];
            cur_road = 0;
            rest = -1;
        }
        
        void make_way(int f) {
            std::vector<int> visited(juncs.size(), 0);
            std::vector<int> dist(juncs.size(), -1);
            std::vector<int> prev(juncs.size(), -1);
            dist[f] = 0;
            prev[f] = f;
            int min_dist = -1, min_index = -2;

            while (min_index != -1) {
                min_dist = -1;
                min_index = -1;

                for (int i = 0; i < juncs.size(); ++i) {
                    if (visited[i] == 0 && dist[i] != -1 && (min_dist == -1 || dist[i] < min_dist)) {
                        min_dist = dist[i];
                        min_index = i;
                    }
                }

                if (min_dist != -1) {
                    for (int i = 0; i < graph.size(); ++i) {
                        if (graph[min_index][i] > 0) {
                            int tmp = min_dist + graph[min_index][i];
                            if (tmp < dist[i] || dist[i] == -1) {
                                dist[i] = tmp;
                                prev[i] = min_index;
                            }
                        }
                    }
                    visited[min_index] = 1;
                }
            }

            for (int i = 0; i < juncs.size(); ++i) {
                if (best_ways[f][i].empty()) {
                    std::vector<BW> b_w;
                    BW tmp;
                    tmp.road_num = i;
                    b_w.push_back(tmp);
                    int cur = i;

                    while (cur != f) {
                        cur = prev[cur];
                        tmp.road_num = cur;
                        b_w.push_back(tmp);
                    }

                    std::reverse(b_w.begin(), b_w.end());
                    
                    for (int j = 0; j < b_w.size() - 1; ++j) {
                        for (int k = 0; k < roads.size(); ++k) {
                            if (roads[k].from == b_w[j].road_num && roads[k].to == b_w[j + 1].road_num) {
                                tmp.road_num = k;
                                best_ways[f][i].push_back(tmp);
                            }
                        }
                    }
                }
            }
        } 
};

int main() {
    srand(time(0));
    int num_of_roads, num_of_juncs;
    std::cin >> num_of_roads >> num_of_juncs;
    std::vector<int> start_points;
    graph.resize(num_of_juncs, std::vector<double>(num_of_juncs, -1));
    for (int i = 0; i < num_of_roads; ++i) {
        int id, fr, t, l, ln, ms;
        std::cin >> id >> fr >> t >> l >> ln >> ms;
        ms = std::round((double)ms / 7.2);
        Road tmp(id, fr, t, l, ln, ms);
        roads.push_back(tmp);
        graph[fr][t] = ln/ms;
    }

    juncs.resize(num_of_juncs);
    best_ways.resize(num_of_juncs, std::vector<std::vector<BW>>(num_of_juncs));
    for (int i = 0; i < num_of_juncs; ++i) {
        std::cin >> juncs[i].j_id >> juncs[i].start;

        if (juncs[i].start == 1) {
            start_points.push_back(i);
        }
        
        for (int j = 0; j < num_of_roads; ++j) {
            std::pair<int, int> ft = roads[j].get_ft();
            if (ft.first == juncs[i].j_id || ft.second == juncs[i].j_id) {
                juncs[i].roads.push_back(roads[j]);
            }
        }

        int num_of_conns;
        std::cin >> num_of_conns;
        for (int i = 1; i <= num_of_conns; ++i) {
            Connection tmp;
            std::cout << "\nConnection " << i << ":\n";
            std::cout << "road id 1: ";
            std::cin >> tmp.r_id_1;
            std::cout << "\nlane of road 1: ";
            std::cin >> tmp.line_1;
            std::cout << "\nroad id 2: ";
            std::cin >> tmp.r_id_2;
            std::cout << "\nlane of road 2: ";
            std::cin >> tmp.line_2;
            juncs[i].conns.push_back(tmp);
        }
    }

    std::cout << "\nwork:\n";

    int turn = 0;
    while (1) { 
        // move cars
        std::vector<std::pair<int, int>> try_again; // <num of road, num of car>
        for (int i = 0; i < roads.size(); ++i) {
            for (int j = 0; j < roads[i].road_cars.size(); ++j) {
                int action_points;
                if (roads[i].road_cars[j].rest == -1) {
                    if (roads[i].road_cars[j].speed < roads[i].max_speed) {
                        if (roads[i].road_cars[j].speed + roads[i].road_cars[j].accel <= roads[i].max_speed) {
                            roads[i].road_cars[j].speed += roads[i].road_cars[j].accel;
                            action_points = roads[i].road_cars[j].speed;
                        } else {
                            roads[i].road_cars[j].speed = roads[i].max_speed - roads[i].road_cars[j].speed;
                        }
                    } else if (roads[i].road_cars[j].speed > roads[i].max_speed) {
                        roads[i].road_cars[j].speed = roads[i].max_speed;
                        action_points = roads[i].max_speed; 
                    } else {
                        action_points = roads[i].max_speed;
                    }
                } else {
                    action_points = roads[i].road_cars[j].rest;
                }

                if (roads[i].road_cars[j].road_block + action_points < roads[i].len) {
                    int p = action_points;
                    while (p > 0) {
                        //std::cout << roads[i].road_cars[j].road_block + 1 << " " << roads[i].len << " " << roads[i].free_pos[0][roads[i].road_cars[j].road_block + 1] << "\n";
                        int cur = roads[i].road_cars[j].road_block, size = roads[i].road_cars[j].size;
                        if (cur + size + 1 < roads[i].len && roads[i].free_pos[0][cur + size + 1] == 1) {
                            roads[i].free_pos[0][cur + size + 1] = 0;
                            roads[i].free_pos[0][cur - size] = 1;
                            roads[i].road_cars[j].road_block++;
                            --p;
                        } else {
                            break;
                        }
                    }

                    if (p > 0) {
                        roads[i].road_cars[j].rest = p;
                        try_again.push_back(std::make_pair(i, j));
                    } else {
                        roads[i].road_cars[j].rest = -1;
                    }
                } else { 
                    if (roads[i].road_cars[j].cur_road + 1 == roads[i].road_cars[j].way.size()) {
                        for (int b = -roads[i].road_cars[j].size; b <= roads[i].road_cars[j].size; ++b) {
                            if (roads[i].road_cars[j].road_block + b < roads[i].len) {
                                roads[i].free_pos[0][roads[i].road_cars[j].road_block + b] = 1;
                            }
                        }
                        roads[i].road_cars.erase(roads[i].road_cars.begin() + j);
                        --j;
                    } else {
                        int new_road = roads[i].road_cars[j].way[roads[i].road_cars[j].cur_road + 1].road_num;
                        int flag = 1;
                        for (int b = 0; b <= 2 * roads[i].road_cars[j].size; ++b) {
                            if (roads[new_road].free_pos[0][b] == 0) {
                                flag = 0;
                                break;
                            }
                        }
                        if (flag == 1) {
                            for (int b = -roads[i].road_cars[j].size; b <= roads[i].road_cars[j].size; ++b) {
                                if (roads[i].road_cars[j].road_block + b < roads[i].len) {
                                    roads[i].free_pos[0][roads[i].road_cars[j].road_block + b] = 1;
                                }
                            }
                            for (int b = 0; b <= 2 * roads[i].road_cars[j].size; ++b) {
                                roads[new_road].free_pos[0][b] = 0;
                            }
                            roads[i].road_cars[j].cur_road++;
                            roads[i].road_cars[j].road_block = roads[i].road_cars[j].size;
                            roads[i].road_cars[j].rest = roads[i].road_cars[j].road_block + action_points - roads[i].len;
                            roads[new_road].road_cars.push_back(roads[i].road_cars[j]);
                            roads[i].road_cars.erase(roads[i].road_cars.begin() + j);
                            
                            if (new_road < i) {
                                try_again.push_back(std::make_pair(new_road, roads[new_road].road_cars.size() - 1));
                            }
                        } else {
                            int p = roads[i].len - roads[i].road_cars[j].road_block - roads[i].road_cars[j].size;
                            while (p > 0) {
                                int cur = roads[i].road_cars[j].road_block, size = roads[i].road_cars[j].size;
                                if (cur + size + 1 < roads[i].len && roads[i].free_pos[0][cur + size + 1] == 1) {
                                    roads[i].free_pos[0][cur + size + 1] = 0;
                                    roads[i].free_pos[0][cur - size] = 1;
                                    roads[i].road_cars[j].road_block++;
                                    --p;
                                } else {
                                    break;
                                }
                            }
                            
                            if (p > 0) {
                                roads[i].road_cars[j].rest = p;
                                try_again.push_back(std::make_pair(i, j));
                            } else {
                                roads[i].road_cars[j].rest = -1;
                            }
                        }
                    }
                }
            }
        }
        
        int changes = 1;
        while (changes == 1) {
            changes = 0;
            for (int i = 0; i < try_again.size(); ++i) {
                int action_points, rn = try_again[i].first, cn = try_again[i].second;
                if (roads[rn].road_cars[cn].rest == -1) {
                    if (roads[rn].road_cars[cn].speed < roads[i].max_speed) {
                        if (roads[rn].road_cars[cn].speed + roads[rn].road_cars[cn].accel <= roads[rn].max_speed) {
                            roads[rn].road_cars[cn].speed += roads[rn].road_cars[cn].accel;
                            action_points = roads[rn].road_cars[cn].speed;
                        } else {
                            roads[rn].road_cars[cn].speed = roads[rn].max_speed - roads[rn].road_cars[cn].speed;
                        }
                    } else if (roads[rn].road_cars[cn].speed > roads[rn].max_speed) {
                        roads[rn].road_cars[cn].speed = roads[rn].max_speed;
                        action_points = roads[rn].max_speed; 
                    } else {
                       action_points = roads[rn].max_speed;
                    }
                } else {
                    action_points = roads[rn].road_cars[cn].rest;
                }
                
                if (roads[rn].road_cars[cn].road_block + action_points < roads[rn].len) {
                    int p = action_points;
                    while (p > 0) {
                        int cur = roads[rn].road_cars[cn].road_block, size = roads[rn].road_cars[cn].speed;
                        if (cur + size + 1 < roads[rn].len && roads[rn].free_pos[0][cur + size + 1] == 1) {
                            roads[rn].free_pos[0][cur + size + 1] = 0;
                            roads[rn].free_pos[0][cur - size] = 1;
                            roads[rn].road_cars[cn].road_block++;
                            --p;
                        } else {
                            break;
                        }
                    }
                    
                    if (p > 0 && changes == 1) {
                        roads[rn].road_cars[cn].rest = p;
                    } else {
                        roads[rn].road_cars[cn].rest = -1;
                    }
                } else {
                    if (roads[rn].road_cars[cn].cur_road + 1 == roads[rn].road_cars[cn].way.size()) { 
                        for (int b = -roads[rn].road_cars[cn].size; b <= roads[rn].road_cars[cn].size; ++b) {
                            if (roads[rn].road_cars[cn].road_block + b < roads[rn].len) {
                                roads[rn].free_pos[0][roads[rn].road_cars[cn].road_block + b] = 1;
                            }
                        }
                        roads[rn].road_cars.erase(roads[rn].road_cars.begin() + cn);
                        changes = 1;
                    } else {
                        int new_road = roads[rn].road_cars[cn].way[roads[rn].road_cars[cn].cur_road + 1].road_num;
                        int flag = 1;
                        for (int b = 0; b <= 2 * roads[rn].road_cars[cn].size; ++b) {
                            if (roads[new_road].free_pos[0][b] == 0) {
                                flag = 0;
                                break;
                            }
                        }
                        if (flag == 1) {
                            for (int b = -roads[rn].road_cars[cn].size; b <= roads[rn].road_cars[cn].size; ++b) {
                                if (roads[rn].road_cars[cn].road_block + b < roads[rn].len) {
                                    roads[i].free_pos[0][roads[rn].road_cars[cn].road_block + b] = 1;
                                }
                            }
                            for (int b = 0; b <= 2 * roads[rn].road_cars[cn].size; ++b) {
                                roads[new_road].free_pos[0][b] = 0;
                            }
                            roads[rn].road_cars[cn].cur_road++;
                            roads[rn].road_cars[cn].road_block = roads[rn].road_cars[cn].size;
                            roads[rn].road_cars[cn].rest = roads[rn].road_cars[rn].road_block + action_points - roads[rn].len;
                            roads[new_road].road_cars.push_back(roads[rn].road_cars[cn]);
                            roads[rn].road_cars.erase(roads[rn].road_cars.begin() + cn);
                            
                            try_again.push_back(std::make_pair(new_road, roads[new_road].road_cars.size() - 1));
                            changes = 1;
                        } else {
                            int p = roads[rn].len - roads[rn].road_cars[cn].road_block;
                            while (p > 0) {
                                int cur = roads[rn].road_cars[cn].road_block, size = roads[rn].road_cars[cn].size;
                                if (cur + size + 1 < roads[rn].len && roads[i].free_pos[0][cur + size + 1] == 1) {
                                    roads[rn].free_pos[0][cur + size + 1] = 0;
                                    roads[rn].free_pos[0][cur - size] = 1;
                                    roads[rn].road_cars[cn].road_block++;
                                    --p;
                                } else {
                                    break;
                                }
                            }
                            
                            if (p > 0 && changes == 1) {
                                roads[rn].road_cars[cn].rest = p;
                            } else {
                                roads[rn].road_cars[cn].rest = -1;
                            }
                        }
                    }
                }
            }
        }

        // create cars
        if (turn % 2 == 0) {
            int f = 1, t = 1;
            while (f == t) {
                f = start_points[random() % start_points.size()];
                t = start_points[random() % start_points.size()];
            }

            Car tmp(0, f, t);
            if (roads[tmp.way[0].road_num].free_pos[0][2] == 1) {
                roads[tmp.way[0].road_num].road_cars.push_back(tmp);
                for (int p = 0; p < 5; ++p) {
                    roads[tmp.way[0].road_num].free_pos[0][p] = 0;
                }
            }
        }

        try_again.clear();

        sleep(1);
        ++turn;
    }
}
