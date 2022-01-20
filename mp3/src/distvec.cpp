#include <iostream>
#include <unordered_map>
#include <fstream>
#include <set>
#include <vector>

using namespace std;

unordered_map<int, unordered_map<int, int>> topo;
set<int> nodeSet;
typedef struct message {
    int src;
    int dest;
    string msg;

    message(int src, int dest, string msg) : src(src), dest(dest), msg(msg) {}
} message;
vector<message> messageVector;

string topoFile, msgFile, changeFile;
ofstream outputFile;
unordered_map<int, unordered_map<int, pair<int, int> > > forwardMap; // src, des, next， cost

bool findItemInMap(int x, int y, unordered_map<int, unordered_map<int, int> > map) {
    auto it = map.find(x);
    auto res = it->second.find(y);
    return !(res == it->second.end());
}

void getTopo(string file, unordered_map<int, unordered_map<int, int>> &topo,
             unordered_map<int, unordered_map<int, pair<int, int>>> &forwardMap) {
ifstream topoin;
topoin.open(file, ios::in);
if (!topoin) {
cout << "open file error!" << endl;
}

int src, dest, cost;
while (topoin >> src >> dest >> cost) {
topo[src][dest] = cost;
topo[dest][src] = cost;
if (nodeSet.find(src) == nodeSet.end()) {
nodeSet.insert(src);
}
if (nodeSet.find(dest) == nodeSet.end()) {
nodeSet.insert(dest);
}
}
for (auto k = nodeSet.begin(); k != nodeSet.end(); ++k) {
src = *k;
for (auto i = nodeSet.begin(); i != nodeSet.end(); ++i) {
dest = *i;
if (src == dest) {
topo[src][dest] = 0;
}
if (!findItemInMap(src, dest, topo)) {
topo[src][dest] = -999;
}
forwardMap[src][dest] = make_pair(dest, topo[src][dest]);
}
}
topoin.close();
}

void initForwardTable() {
    int src, dest;
    for (auto k = nodeSet.begin(); k != nodeSet.end(); ++k) {
        src = *k;
        for (auto i = nodeSet.begin(); i != nodeSet.end(); ++i) {
            dest = *i;
            forwardMap[src][dest] = make_pair(dest, topo[src][dest]);
        }
    }
}

void getForwardMap(unordered_map<int, unordered_map<int, pair<int, int>>> &forwardMap) {
    int num = nodeSet.size();
    int src, dest, min, minCost;
    for (int i = 0; i < num; ++i) {
        for (auto j = nodeSet.begin(); j != nodeSet.end(); ++j) {
            src = *j;
            for (auto k = nodeSet.begin(); k != nodeSet.end(); ++k) {
                dest = *k;
                min = forwardMap[src][dest].first;
                minCost = forwardMap[src][dest].second;
                for (auto m = nodeSet.begin(); m != nodeSet.end(); ++m) {
                    if (topo[src][*m] >= 0 && forwardMap[*m][dest].second >= 0) {
                        int tempCost = topo[src][*m] + forwardMap[*m][dest].second;
                        if (minCost < 0 || tempCost < minCost) {
                            min = *m;
                            minCost = tempCost;
                            }
                    }
                 }
            forwardMap[src][dest] = make_pair(min, minCost);
            }
        }
    }
    outputFile << endl;
    for (auto l = nodeSet.begin(); l != nodeSet.end(); ++l) {
        src = *l;
        for (auto i = nodeSet.begin(); i != nodeSet.end(); ++i) {
            dest = *i;
            outputFile << dest << " " << forwardMap[src][dest].first << " " << forwardMap[src][dest].second << endl;
            }
        }
}

void getMessage(string file) {
    ifstream msgfile(file);

    string line, msg;
    if (msgfile.is_open()) {
        while (getline(msgfile, line)) {
            int src, dest;
            sscanf(line.c_str(), "%d %d %*s", &src, &dest);
            msg = line.substr(line.find(" "));
            msg = msg.substr(line.find(" ") + 1);
            message newMessage(src, dest, msg);
            messageVector.push_back(newMessage);
        }
    }
    msgfile.close();
}

void sendMessage() {
    int src, dest, cost, next;
    for (int i = 0; i < messageVector.size(); ++i) {
        src = messageVector[i].src;
        dest = messageVector[i].dest;
        next = src;
        outputFile << "from " << src << " to " << dest << " cost ";
        cost = forwardMap[src][dest].second;
        if (cost == -999) {
            outputFile << "infinite hops unreachable ";
        } else {
            outputFile << cost << " hops ";
            while (next != dest) {
                outputFile << next << " ";
                next = forwardMap[next][dest].first;
            }
        }
        outputFile << "message " << messageVector[i].msg << endl;
    }
}

void change(string file) {
    ifstream change(file);
    int src, dest, cost;
    if (change.is_open()) {
        while (change >> src >> dest >> cost) {
            topo[src][dest] = cost;
            topo[dest][src] = cost;
            initForwardTable();
            getForwardMap(forwardMap);
            sendMessage();
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: ./linkstate topoFile msgFile changeFile\n");
        return -1;
    }
    topoFile = argv[1];
    msgFile = argv[2];
    changeFile = argv[3];
    getTopo(topoFile, topo, forwardMap);
    outputFile.open("output.txt");
    getForwardMap(forwardMap);
    getMessage(msgFile);
    sendMessage();
    change(changeFile);

    outputFile.close();
    return 0;
}