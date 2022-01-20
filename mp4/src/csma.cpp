#include <iostream>
#include <unordered_map>
#include <fstream>
#include <set>
#include <vector>
#include <iomanip>

using namespace std;

class node {
public:
    int backoff;
    int collisionNum;
    int nodeId;

    node(int id) {
        this->nodeId = id;
        this->backoff = 0;
        this->collisionNum = 0;
    }

    void setRandom(vector<int> &R, int time) {
        this->backoff = (this->nodeId + time) % (R[this->collisionNum]);
    }
};

// number of nodes N
// length of packet L
// initial random number R
// max retransmission attemp times M
// global time T
int N, L, R, M, T;
vector<int> Rvec;
ofstream fpOut("output.txt");


vector<int> findNodesToSend(vector<node *> &nodeVec) {
    vector<int> res;
    for (int i = 0; i < nodeVec.size(); i++) {
        if (nodeVec[i]->backoff == 0) {
            res.push_back(i);
        }
    }
    return res;
}

void readFileAndInit(string filename) {
    ifstream in(filename);
    char c;
    in >> c >> N >> c >> L >> c >> M >> c;

    while (in.get() != '\n') {
        in >> R;
        Rvec.push_back(R);
    }
    in >> c >> T;
}

void simulate(int n) {

    vector < node * > nodeVec(n);
    for (int i = 0; i < n; i++) {
        nodeVec[i] = new node(i);
        nodeVec[i]->setRandom(Rvec, 0);
    }
    // variables for statistics
    int curOccupyNode;
    int packetSentNum = 0;
    int endtime = 0;
    for (int time = 0; time < T; time++) {
        // 所有要发送的节点id
        vector<int> nodesToSend = findNodesToSend(nodeVec);
        int sendNum = nodesToSend.size();
        cout << "=================================" << endl;
        cout << "At time " << time << " send nodes number: " << sendNum << endl;
        for (int i = 0; i < sendNum; ++i) {
            cout << nodesToSend[i] << endl;
        }
        for (int i = 0; i < nodeVec.size(); ++i) {
            cout << "node " << i << ", backoff = " << nodeVec[i]->backoff << endl;
        }
        // 没有节点发送，所有节点的backoff--
        if (sendNum == 0) {
            for (int i = 0; i < n; i++) {
                nodeVec[i]->backoff--;
            }
            // 存在冲突
        } else if (sendNum > 1) {
            for (int i = 0; i < sendNum; i++) {
                nodeVec[nodesToSend[i]]->collisionNum++;
                if (nodeVec[nodesToSend[i]]->collisionNum == M) {
                    nodeVec[nodesToSend[i]]->collisionNum = 0;
                }
                nodeVec[nodesToSend[i]]->setRandom(Rvec, time + 1);
            }
            // 发送package
        } else {
            curOccupyNode = nodesToSend[0];
            cout << "occupy node:" << curOccupyNode << endl;
            if (time + L <= T) {
                time += L - 1;
                packetSentNum++;
                nodeVec[curOccupyNode]->collisionNum = 0;
                nodeVec[curOccupyNode]->setRandom(Rvec, time + 1);
            } else {
                endtime = T - time;
                time = T;
            }
        }
    }

    cout << packetSentNum << endl;
    cout << T << endl;
    cout << fixed << setprecision(2) << (packetSentNum * L + endtime) * 1.0 / T << endl;
    fpOut << fixed << setprecision(2) << (packetSentNum * L + endtime) * 1.0 / T << endl;
}

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage: ./csma input.txt\n");
        return -1;
    }

    readFileAndInit(argv[1]);

    simulate(N);
    fpOut.close();

    return 0;
}