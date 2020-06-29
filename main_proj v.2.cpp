#include <algorithm>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>
#include <random>
#include <unistd.h>
#include <ctime>

int global_num = 0;
std::vector<std::vector<std::pair<int, double>>> graph;

struct Connection {
    int r_id_1;
    int line_1;
    int r_id_2;
    int line_2;
    int signal = 1;
    int cur_time;
    int green = 1000000;
    int red = 0;
};

struct BW {
    int road_num = 0;
    std::vector<int> lines;
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
        int tl_flag;
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

        int check(int j, int from, int to) {
            for (int i = 0; i < juncs[j].conns.size(); ++i) {
                if (juncs[j].conns[i].r_id_1 == from && juncs[j].conns[i].r_id_2 == to) {
                    return 1;
                }
            }
            return 0;
        }
        
        void make_way(int f) {
            std::vector<int> visited(juncs.size(), 0);
            std::vector<int> dist(juncs.size(), -1);
            std::vector<std::pair<int, int>> prev(juncs.size(), std::make_pair(-1, -1));
            dist[f] = 0;
            prev[f] = std::make_pair(f, -1);
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
                        if (graph[min_index][i].second > 0) {
                            int tmp = min_dist + graph[min_index][i].second;
                            if ((tmp < dist[i] || dist[i] == -1) && (prev[min_index].second == -1 || check(min_index, prev[min_index].second, graph[min_index][i].first))) {
                                dist[i] = tmp;
                                prev[i] = std::make_pair(min_index, graph[min_index][i].first);
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
                        cur = prev[cur].first;
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

            for (int i = 0; i < best_ways[f].size(); ++i) {
                if (best_ways[f][i].size() == 1) {
                    for (int j = 0; j < roads[best_ways[f][i][0].road_num].num_of_lines; ++j) {
                        best_ways[f][i][0].lines.push_back(j);
                    }
                } else if (best_ways[f][i].size() > 1) {
                    for (int j = 0; j < best_ways[f][i].size() - 1; ++j) {
                        int jn = roads[best_ways[f][i][j].road_num].to;
                        int r1 = best_ways[f][i][j].road_num;
                        int r2 = best_ways[f][i][j + 1].road_num;
                        for (int c = 0; c < juncs[jn].conns.size(); ++c) {
                            if (juncs[jn].conns[c].r_id_1 == r1 && juncs[jn].conns[c].r_id_2 == r2) {
                                best_ways[f][i][j].lines.push_back(juncs[jn].conns[c].line_1);
                            }
                        }
                    }
                }
            }
        } 
};

int line_check(int l, BW b) {
    for (int i = 0; i < b.lines.size(); ++i) {
        if (b.lines[i] == l) {
            return 0;
        }
    }
    return 1;
}

int traffic_light_check(int j, int rf, int lf, int rt, int lt) {
    for (int i = 0; i < juncs[j].conns.size(); ++i) {
        Connection tmp = juncs[j].conns[i];
        if (tmp.r_id_1 == rf && tmp.r_id_2 == rt && tmp.line_1 == lf && tmp.line_2 == lt) {
            return tmp.signal;
        }
    }
}

int target(int l, BW b) {
    int c = 100;
    for (int i = 0; i < b.lines.size(); ++i) {
        if (abs(b.lines[i] - l) < abs(c)) {
            c = b.lines[i] - l;
        }
    }
    if (c == 100) {
        return 0;
    } else {
        return c;
    }
}

int get_new_line(int j, int r1, int l, int r2) {
    for (int i = 0; i < juncs[j].conns.size(); ++i) {
        if (juncs[j].conns[i].r_id_1 == r1 && juncs[j].conns[i].r_id_2 == r2 && juncs[j].conns[i].line_1 == l) {
            return juncs[j].conns[i].line_2;
        }
    }
}

int main() {
    int test = 0;
    int trans_proc = 0;
    std::cin >> test;
    srand(time(0));
    int num_of_roads, num_of_juncs;
    std::cin >> num_of_roads >> num_of_juncs;
    std::vector<int> start_points;
    graph.resize(num_of_juncs, std::vector<std::pair<int, double>>(num_of_juncs, std::make_pair(-1, -1)));
    for (int i = 0; i < num_of_roads; ++i) {
        int id, fr, t, l, ln, ms;
        std::cin >> id >> fr >> t >> l >> ln >> ms;
        ms = std::round((double)ms / 7.2);
        Road tmp(id, fr, t, l, ln, ms);
        roads.push_back(tmp);
        graph[fr][t] = std::make_pair(id, (double)ln / (double)ms);
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

        std::cin >> juncs[i].tl_flag;

        int num_of_conns;
        std::cin >> num_of_conns;
        for (int j = 0; j < num_of_conns; ++j) {
            Connection tmp;
            std::cin >> tmp.r_id_1;
            std::cin >> tmp.line_1;
            std::cin >> tmp.r_id_2;
            std::cin >> tmp.line_2;

            if (juncs[i].tl_flag == 1) {
                std::cin >> tmp.signal;
                std::cin >> tmp.green;
                std::cin >> tmp.red;
            }

            juncs[i].conns.push_back(tmp);
        }

    }

    std::cin >> trans_proc;
    std::cout << "\nwork:\n";

    int turn = 0;
    while (1) {
        turn %= 100000;
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
                            roads[i].road_cars[j].speed = roads[i].max_speed;
                            action_points = roads[i].road_cars[j].speed;
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
                    int size = roads[i].road_cars[j].size, line = roads[i].road_cars[j].line;
                    int target_line = target(line, roads[i].road_cars[j].way[roads[i].road_cars[j].cur_road]);
                        if (target_line > 0) {
                            if (turn % 2 == 1) {
                                int flag = 1;
                                for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                    if (roads[i].road_cars[j].road_block + s < roads[i].len && roads[i].free_pos[line + 1][roads[i].road_cars[j].road_block + s] == 0) {
                                        flag = 0;
                                        break;
                                    }
                                }
                                if (flag == 1) {
                                    for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                        if (roads[i].road_cars[j].road_block + s < roads[i].len) {
                                            roads[i].free_pos[line + 1][roads[i].road_cars[j].road_block + s] = 0;
                                        }
                                    }
                                    for (int s = -size; s <= size; ++s) {
                                        if (roads[i].road_cars[j].road_block + s < roads[i].len) {
                                            roads[i].free_pos[line][roads[i].road_cars[j].road_block + s] = 1;
                                        }
                                    }
                                    roads[i].road_cars[j].road_block += std::min(2 * size + 1, roads[i].len - 1);
                                    roads[i].road_cars[j].line++;
                                }
                            }
                        } else {
                            if (turn % 2 == 0) {
                                int flag = 1;
                                for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                    if (roads[i].road_cars[j].road_block + s < roads[i].len && roads[i].free_pos[line - 1][roads[i].road_cars[j].road_block + s] == 0) {
                                        flag = 0;
                                        break;
                                    }
                                }
                                if (flag == 1) {
                                    for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                        if (roads[i].road_cars[j].road_block + s < roads[i].len) {
                                            roads[i].free_pos[line - 1][roads[i].road_cars[j].road_block + s] = 0;
                                        }
                                    }
                                    for (int s = -size; s <= size; ++s) {
                                        if (roads[i].road_cars[j].road_block + s < roads[i].len) {
                                            roads[i].free_pos[line][roads[i].road_cars[j].road_block + s] = 1;
                                        }
                                    }

                                    roads[i].road_cars[j].road_block += std::min(2 * size + 1, roads[i].len - 1);
                                    roads[i].road_cars[j].line--;
                                }
                            }
                        }
                        roads[i].road_cars[j].rest = -1;
                    } else {
                        int p = action_points;
                        while (p > 0) { 
                            int cur = roads[i].road_cars[j].road_block;
                            if (cur + size + 1 < roads[i].len && roads[i].free_pos[line][cur + size + 1] == 1) {
                                roads[i].free_pos[line][cur + size + 1] = 0;
                                roads[i].free_pos[line][cur - size] = 1;
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
                } else {
                    if (roads[i].road_cars[j].cur_road + 1 == roads[i].road_cars[j].way.size()) {
                        for (int b = -roads[i].road_cars[j].size; b <= roads[i].road_cars[j].size; ++b) {
                            if (roads[i].road_cars[j].road_block + b < roads[i].len) {
                                roads[i].free_pos[roads[i].road_cars[j].line][roads[i].road_cars[j].road_block + b] = 1;
                            }
                        }
                        roads[i].road_cars.erase(roads[i].road_cars.begin() + j);
                        --j;
                    } else {
                        int new_road = roads[i].road_cars[j].way[roads[i].road_cars[j].cur_road + 1].road_num;
                        int new_line = get_new_line(roads[i].to, i, roads[i].road_cars[j].line, new_road);
                        int flag = 1;
                        
                        for (int b = 0; b <= 2 * roads[i].road_cars[j].size; ++b) {
                            if (roads[new_road].free_pos[roads[i].road_cars[j].line][b] == 0) {
                                flag = 0;
                                break;
                            }
                        }

                        if (flag != 0 && traffic_light_check(roads[i].to, i, roads[i].road_cars[j].line, new_road, new_line) == 0) {
                            flag = 0;
                        }
                        
                        if (flag == 1) {
                            for (int b = -roads[i].road_cars[j].size; b <= roads[i].road_cars[j].size; ++b) {
                                if (roads[i].road_cars[j].road_block + b < roads[i].len) {
                                    roads[i].free_pos[roads[i].road_cars[j].line][roads[i].road_cars[j].road_block + b] = 1;
                                }
                            }
                            for (int b = 0; b <= 2 * roads[i].road_cars[j].size; ++b) {
                                roads[new_road].free_pos[new_line][b] = 0;
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
                                int line = roads[i].road_cars[j].line;
                                if (cur + size + 1 < roads[i].len && roads[i].free_pos[line][cur + size + 1] == 1) {
                                    roads[i].free_pos[line][cur + size + 1] = 0;
                                    roads[i].free_pos[line][cur - size] = 1;
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
                            roads[rn].road_cars[cn].speed = roads[rn].max_speed;
                            action_points = roads[rn].road_cars[cn].speed;
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
                    int size = roads[rn].road_cars[cn].size, line = roads[rn].road_cars[cn].line;
                    int target_line = target(line, roads[rn].road_cars[cn].way[roads[rn].road_cars[cn].cur_road]);
                    if (target_line != 0) {
                        if (target_line > 0) {
                            if (turn % 2 == 1) {
                                int flag = 1;
                                for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                    if (roads[rn].road_cars[cn].road_block + s < roads[rn].len && roads[rn].free_pos[line + 1][roads[rn].road_cars[cn].road_block + s] == 0) {
                                        flag = 0;
                                        break;
                                    }
                                }
                                if (flag == 1) {
                                    for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                        if (roads[rn].road_cars[cn].road_block + s < roads[rn].len) {
                                            roads[rn].free_pos[line + 1][roads[rn].road_cars[cn].road_block + s] = 0;
                                        }
                                    }
                                    for (int s = -size; s <= size; ++s) {
                                        if (roads[rn].road_cars[cn].road_block + s < roads[rn].len) {
                                            roads[rn].free_pos[line][roads[rn].road_cars[cn].road_block + s] = 1;
                                        }
                                    }
                                    roads[rn].road_cars[cn].road_block += std::min(2 * size + 1, roads[rn].len - 1);
                                    roads[rn].road_cars[cn].line++;
                                }
                            }
                        } else {
                            if (turn % 2 == 0) {
                                int flag = 1;
                                for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                    if (roads[rn].road_cars[cn].road_block + s < roads[rn].len && roads[rn].free_pos[line - 1][roads[rn].road_cars[cn].road_block + s] == 0) {
                                        flag = 0;
                                        break;
                                    }
                                }
                                if (flag == 1) {
                                    for (int s = size + 1; s <= 3 * size + 1; ++s) {
                                        if (roads[rn].road_cars[cn].road_block + s < roads[rn].len) {
                                            roads[rn].free_pos[line - 1][roads[rn].road_cars[cn].road_block + s] = 0;
                                        }
                                    }
                                    for (int s = -size; s <= size; ++s) {
                                        if (roads[rn].road_cars[cn].road_block + s < roads[rn].len) {
                                            roads[rn].free_pos[line][roads[rn].road_cars[cn].road_block + s] = 1;
                                        }
                                    }
                                    roads[rn].road_cars[cn].road_block += std::min(2 * size + 1, roads[i].len - 1);
                                        roads[rn].road_cars[cn].line--;
                                }
                            }
                        }
                        roads[rn].road_cars[cn].rest = -1;
                    } else {
                        int p = action_points;
                        while (p > 0) {
                            int cur = roads[rn].road_cars[cn].road_block;
                            if (cur + size + 1 < roads[rn].len && roads[rn].free_pos[line][cur + size + 1] == 1) {
                                roads[rn].free_pos[line][cur + size + 1] = 0;
                                roads[rn].free_pos[line][cur - size] = 1;
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
                } else {
                    if (roads[rn].road_cars[cn].cur_road + 1 == roads[rn].road_cars[cn].way.size()) { 
                        for (int b = -roads[rn].road_cars[cn].size; b <= roads[rn].road_cars[cn].size; ++b) {
                            if (roads[rn].road_cars[cn].road_block + b < roads[rn].len) {
                                roads[rn].free_pos[roads[rn].road_cars[cn].line][roads[rn].road_cars[cn].road_block + b] = 1;
                            }
                        }
                        roads[rn].road_cars.erase(roads[rn].road_cars.begin() + cn);
                        changes = 1;
                    } else {
                        int new_road = roads[rn].road_cars[cn].way[roads[rn].road_cars[cn].cur_road + 1].road_num;
                        int new_line = get_new_line(roads[rn].to, rn, roads[rn].road_cars[cn].line, new_road);
                        int flag = 1;
                        
                        for (int b = 0; b <= 2 * roads[rn].road_cars[cn].size; ++b) {
                            if (roads[new_road].free_pos[roads[rn].road_cars[cn].line][b] == 0) {
                                flag = 0;
                                break;
                            }
                        }
                        
                        if (flag != 0 && traffic_light_check(roads[rn].to, rn, roads[rn].road_cars[cn].line, new_road, new_line) == 0) {
                            flag = 0;
                        }

                        if (flag == 1) {
                            for (int b = -roads[rn].road_cars[cn].size; b <= roads[rn].road_cars[cn].size; ++b) {
                                if (roads[rn].road_cars[cn].road_block + b < roads[rn].len) {
                                    roads[i].free_pos[roads[rn].road_cars[cn].line][roads[rn].road_cars[cn].road_block + b] = 1;
                                }
                            }
                            for (int b = 0; b <= 2 * roads[rn].road_cars[cn].size; ++b) {
                                roads[new_road].free_pos[new_line][b] = 0;
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
                                int line = roads[rn].road_cars[cn].line;
                                if (cur + size + 1 < roads[rn].len && roads[i].free_pos[line][cur + size + 1] == 1) {
                                    roads[rn].free_pos[line][cur + size + 1] = 0;
                                    roads[rn].free_pos[line][cur - size] = 1;
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

            if (random() % 100 <= trans_proc) {
                Car tmp(1, f, t);
                if (!best_ways[f][t].empty()) {
                    int rand_line = random() % roads[tmp.way[0].road_num].num_of_lines;
                    int flag = 1;

                    for (int p = 0; p < 2 * tmp.size + 1; ++p) {
                        if (roads[tmp.way[0].road_num].free_pos[rand_line][p] == 0) {
                            flag = 0;
                            break;
                        }
                    }

                    if (flag == 1) {
                        tmp.line = rand_line;
                        roads[tmp.way[0].road_num].road_cars.push_back(tmp);
                        for (int p = 0; p < 2 * tmp.size + 1; ++p) {
                            roads[tmp.way[0].road_num].free_pos[rand_line][p] = 0;
                        }
                    }
                }
            } else {
                Car tmp(0, f , t);
                if (!best_ways[f][t].empty()) {
                    int rand_line = random() % roads[tmp.way[0].road_num].num_of_lines;
                    int flag = 1;

                    for (int p = 0; p < 2 * tmp.size + 1; ++p) {
                        if (roads[tmp.way[0].road_num].free_pos[rand_line][p] == 0) {
                            flag = 0;
                            break;
                        }
                    }

                    if (flag == 1) {
                        tmp.line = rand_line;
                        roads[tmp.way[0].road_num].road_cars.push_back(tmp);
                        for (int p = 0; p < 2 * tmp.size + 1; ++p) {
                            roads[tmp.way[0].road_num].free_pos[rand_line][p] = 0;
                        }
                    }
                }
            }
        }

        for (int j = 0; j < juncs.size(); ++j) {
            for (int c = 0; c < juncs[j].conns.size(); ++c) {
                ++juncs[j].conns[c].cur_time;
                if (juncs[j].conns[c].signal) {
                    if (juncs[j].conns[c].cur_time >= juncs[j].conns[c].green) {
                        juncs[j].conns[c].signal = 0;
                        juncs[j].conns[c].cur_time = 0;
                    }
                } else {
                    if (juncs[j].conns[c].cur_time >= juncs[j].conns[c].red) {
                        juncs[j].conns[c].signal = 1;
                        juncs[j].conns[c].cur_time = 0;
                    }
                }
            }
        }

        try_again.clear();

        if (test == 1) {
		    for (int i = 0; i < best_ways[0][2].size(); ++i) {
		        for (int j = 0; j < best_ways[0][2][i].lines.size(); ++j) {
		            std::cout << best_ways[0][2][i].lines[j] << " ";
		        }
		        std::cout << "\n";
		    }

		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[0].free_pos[0][i];
		    }
		    std::cout << " ";
		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[1].free_pos[0][i];
		    }
		    std::cout << "\n";
		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[0].free_pos[1][i];
		    }
		    std::cout << " ";
		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[1].free_pos[1][i];
		    }
		    std::cout << "\n\n";
		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[3].free_pos[1][49 - i];
		    }
		    std::cout << " ";
		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[2].free_pos[1][49 - i];
		    }
		    std::cout << "\n";
		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[3].free_pos[0][49 - i];
		    }
		    std::cout << " ";
		    for (int i = 0; i < 50; ++i) {
		        std::cout << roads[2].free_pos[0][49 - i];
		    }
		    std::cout << "\n\n\n\n\n\n\n";
        }

        if (test == 2 || test == 3) {
            for (int i = 0; i < 25; ++i) {
                for (int j = 0; j < 50; ++j) {
                    std::cout << " ";
                }
                std::cout << roads[2].free_pos[0][i] << roads[2].free_pos[1][i] << " " << roads[3].free_pos[1][24 - i] << roads[3].free_pos[0][24 - i] << "\n";
            }
            for (int i = 0; i < 50; ++i) {
                std::cout << roads[1].free_pos[0][49 - i];
            }
            std::cout << "     ";
            for (int i = 0; i < 50; ++i) {
                std::cout << roads[4].free_pos[0][49 - i];
            }
            std::cout << "\n";
            for (int i = 0 ; i < 50; ++i) {
                std::cout << roads[1].free_pos[1][49 - i];
            }
            std::cout << "     ";
            for (int i = 0; i < 50; ++i) {
                std::cout << roads[4].free_pos[1][49 - i];
            }
            std::cout << "\n\n";
            for (int i = 0; i < 50; ++i) {
                std::cout << roads[0].free_pos[1][i];
            }
            std::cout << "     ";
            for (int i = 0; i < 50; ++i) {
                std::cout << roads[5].free_pos[1][i];
            }
            std::cout << "\n";
            for (int i = 0 ; i < 50; ++i) {
                std::cout << roads[0].free_pos[0][i];
            }
            std::cout << "     ";
            for (int i = 0 ; i < 50; ++i) {
                std::cout << roads[5].free_pos[0][i];
            }
            std::cout << "\n";
            for (int i = 0; i < 25; ++i) {
                for (int j = 0; j < 50; ++j) {
                    std::cout << " ";
                }
                std::cout << roads[7].free_pos[0][i] << roads[7].free_pos[1][i] << " " << roads[6].free_pos[1][24 - i] << roads[6].free_pos[0][24 - i] << "\n";
            }
            std::cout << "\n\n";
        }

        sleep(1);
        ++turn;
    }
}
