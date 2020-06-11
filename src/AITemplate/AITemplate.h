#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <vector>
#include <utility>
#include <cmath>

bool dbg = true;

class AI : public AIInterface
{
public:
    enum class Mode {
        Offense,
        Defense,
        Random,
        Standard
    };
    friend std::ostream& operator<<(std::ostream& os, Mode& mode){
        switch(mode){
            case Mode::Offense:
                os<<"Offense";
                break;
            case Mode::Defense:
                os<<"Defense";
                break;
            case Mode::Random:
                os<<"Random";
            break;
            case Mode::Standard:
                os<<"Standard";
            break;
        }
        return os;
    }

private:
    int x;
    int y;
    AI::Mode mode; 
    TA::BoardInterface::Tag mytag;

    //for ultimate winning strategy
    std::vector<std::pair<int,int>> waitLine;
    int state;
    int n;
    
public:
    
    //end
    void init(bool order) override
    {
        this->x = rand()%9;
        this->y = rand()%9;
        if (order){
            this->mytag = TA::BoardInterface::Tag::O;
            this->mode = Mode::Standard;//Offense;
            this->x = 1;
            this->y = 1;
            state = -1;
            n = 0;
        }
        else {
            this->mytag = TA::BoardInterface::Tag::X;
            this->mode = Mode::Standard;
        }
    }

    void callbackReportEnemy(int x, int y) override{
        this->x = x;
        this->y = y;
    }
    std::pair<int,int> queryWhereToPut(TA::UltraBoard MainBoard) override{
        if(dbg)std::cout<<"last step:("<<this->x<<" "<<this->y<<")\n";

        //vaild sub Board to put on
        int vaildx = x%3;
        int vaildy = y%3;
        bool confined = true;
        
        if(!MainBoard.sub(vaildx, vaildy).full()){
            confined=true;
            if(dbg)std::cout<<"block vaild:("<<vaildx<<" "<<vaildy<<")\n";
        }
        else{
            confined = false;
            if(dbg)std::cout<<"block vaild:ALL\n";
        }    

        
        //decide where to put: return (retx, rety)
        int retx=0, rety=0;

        //tgtBoard: sub Board it can put on
        TA::Board& tgtBoard = MainBoard.sub(vaildx, vaildy);

        
        //Random, in case times up
        if(!MainBoard.sub(vaildx, vaildy).full()){
            retx = vaildx*3 + rand()%3;
            rety = vaildy*3 + rand()%3;
            while(!MainBoard.isVaild(retx, rety)){
                retx = vaildx*3 + rand()%3;
                rety = vaildy*3 + rand()%3;
            }
            
        }
        else{
            retx = rand()%9;
            rety = rand()%9;
            while(!MainBoard.isVaild(retx, rety)){
                retx = rand()%9;
                rety = rand()%9;
            }
        }
        Mode decision = Mode::Random;
        
        if(mode==Mode::Standard){
            //Standard AI mode
            int points[9][9]={-1000};
            int ultrapoints[9][9]={-1000};
            int maxpnt=0;
            if(confined){
                //in sub Board[vaildx][vaildy]
                //3x3 options
                for(int i=0; i<3; i++){
                    for(int j=0; j<3; j++){
                        points[i][j]=0;
                        ultrapoints[i][j]=0;
                        if(tgtBoard.isPlaceable(i, j)){
                            //empty space
                            //analyze subboard
                            if(!MainBoard.isOccupied(vaildx, vaildy)){
                                //subBoard not occupied yet
                                points[i][j]+=enemyAround(tgtBoard, i, j, this->mytag, 5);
                                points[i][j]+=allyAround(tgtBoard, i, j, this->mytag, 5);
                            
                            }
                            //else{
                                //subBoard occupied
                                //send to occupied or useless
                                if((MainBoard.isOccupied(i,j)) && (!MainBoard.sub(i,j).full())){
                                    points[i][j]+=15;
                                }
                            //}

                            //analyze UltraBoard
                            //bug!!!
                            if(mytag==TA::BoardInterface::Tag::O){
                                ultrapoints[i][j]-=ultraEndnager(MainBoard, i, j, TA::BoardInterface::Tag::X, 1);
                                ultrapoints[i][j] -= subendanger(MainBoard, i, j, TA::BoardInterface::Tag::X, 1);
                            }
                            else if(mytag==TA::BoardInterface::Tag::X){
                                ultrapoints[i][j]-=ultraEndnager(MainBoard, i, j, TA::BoardInterface::Tag::O, 1);
                                ultrapoints[i][j] -= subendanger(MainBoard, i, j, TA::BoardInterface::Tag::O, 1);
                            }

                            //prevent next round still in the same subboard
                            //if(vaildx==i&&vaildy==j)ultrapoints[i][j]-=3;

                            //update (retx, rety) if better
                            int total = points[i][j]+ultrapoints[i][j];
                            if(total>maxpnt || ((total!=-0&&total==maxpnt)&&rand()%2)){
                                maxpnt = total;
                                retx = i;
                                rety = j;
                                decision = Mode::Standard;
                            }
                        }
                    }
                }
            }
            else if(!confined){
                //subBoard full, place anywhere on Ultraboard
                //9x9 options
                for(int i=0; i<9; i++){
                    for(int j=0; j<9; j++){
                        points[i][j]=0;
                        ultrapoints[i][j]=0;
                        if(MainBoard.get(i, j)==TA::BoardInterface::Tag::None){
                            //if placeable

                            //subboard analysis
                            if(MainBoard.isOccupied(i/3, j/3)){
                                //if subBoard not occupied yet
                                points[i][j]+=enemyAround(MainBoard.sub(i/3, j/3), i%3, j%3, this->mytag, 5);
                                points[i][j]+=allyAround(MainBoard.sub(i/3, j/3), i%3, j%3, this->mytag, 5);
                            }
                            //else{
                                if((MainBoard.isOccupied(i/3,j/3)) && (!MainBoard.sub(i/3,j/3).full())){
                                    points[i][j]+=15;
                                }
                            //}

                            //ultraboard analysis
                            //bug!!!
                            if(mytag==TA::BoardInterface::Tag::O){
                                ultrapoints[i][j]-=ultraEndnager(MainBoard, i/3, j/3, TA::BoardInterface::Tag::X, 1);
                                ultrapoints[i][j] -= subendanger(MainBoard, i/3, j/3, TA::BoardInterface::Tag::X, 1);
                            }
                            else if(mytag==TA::BoardInterface::Tag::X){
                                ultrapoints[i][j]-=ultraEndnager(MainBoard, i/3, j/3, TA::BoardInterface::Tag::O, 1);
                                ultrapoints[i][j] -= subendanger(MainBoard, i/3, j/3, TA::BoardInterface::Tag::O, 1);
                            }

                            //update if better
                            int total = points[i][j]+ultrapoints[i][j];
                            if(total>maxpnt || ((total!=0&&total==maxpnt)&&rand()%2)){
                                maxpnt = total;
                                retx = i;
                                rety = j;
                                decision = Mode::Standard;
                            }
                        }
                    }
                }
            }

            //print point for debug
            if(dbg){
                std::cout<<"Decision by MODE::"<<decision<<std::endl;
                std::cout<<"Points analysis:\n";
                
                //index
                for(int i=0; i<9; i++){
                    if(i==0)std::cout<<" ";
                    if(confined){
                        std::cout<<" "<<vaildy*3+i;
                        if(i==2)break;
                    }
                    else std::cout<<" "<<i;
                }
                std::cout<<std::endl;
                //subBoard data
                for(int i=0; i<9; i++){
                    if(confined)std::cout<<vaildx*3+i;
                    else std::cout<<i; 

                    for(int j=0; j<9; j++){
                        if(confined){
                            std::cout<<" "<<points[i][j];
                            if(j==2)break;
                        }
                        else std::cout<<" "<<points[i][j];
                    }
                    std::cout<<"\n";
                    if(confined&&i==2)break;
                }

                for(int i=0; i<9; i++){
                    if(i==0)std::cout<<" ";
                    if(confined){
                        std::cout<<" "<<vaildy*3+i;
                        if(i==2)break;
                    }
                    else std::cout<<" "<<i;
                }
                std::cout<<std::endl;
                //UltraBoard data
                for(int i=0; i<9; i++){
                    if(confined)std::cout<<vaildx*3+i;
                    else std::cout<<i; 

                    for(int j=0; j<9; j++){
                        if(confined){
                            std::cout<<" "<<ultrapoints[i][j];
                            if(j==2)break;
                        }
                        else std::cout<<" "<<ultrapoints[i][j];
                    }
                    std::cout<<"\n";
                    if(confined&&i==2)break;
                }
            }
        }
        else if(mode == Mode::Offense){ //Ultimate Winning Strategy Here
            int validx = vaildx;
            int validy = vaildy;
            if (state == -1) { 
                state = 0;

                retx = 4;
                rety = 4; 
                waitLine.push_back(std::make_pair(1, 1));
            }
            else if (state == 0) {
                TA::Board& offenseBoard = MainBoard.sub(waitLine[n].first, waitLine[n].second);
                if (offenseBoard.full()) {  // if we can not make it to (c, d)
                    state = 2;

                    waitLine.erase(waitLine.begin());   // pop_front
                    waitLine.push_back(std::make_pair(validx, validy));
                    retx = validx * 3 + waitLine[n].first;
                    rety = validy * 3 + waitLine[n].second;
                    n = 0;
                    std::cout << "into state2\n";
                }
                else {
                    retx = validx * 3 + 1;
                    rety = validy * 3 + 1;
                } 
            }
            else if(state == 2){
                if (validx == 1 && validy == 1){
                    validx = abs(2 - waitLine[n].first);
                    validy = abs(2 - waitLine[n].second);
                    waitLine.push_back(std::make_pair(validx,validy));
                     while (MainBoard.get((validx * 3 + waitLine[n].first), (validy * 3 + waitLine[n].second))
                            != TA::BoardInterface::Tag::None){
                            n++;
                            std::cout << "while loop\n";
                        }
                    std::cout << "In21\n";
                } 
                else {
                     TA::Board& offenseBoard = MainBoard.sub(waitLine[n].first, waitLine[n].second);
                     if(offenseBoard.full()){
                        waitLine.erase(waitLine.begin());
                        std::cout << "In22\n";
                     }
                     else{
                        while (MainBoard.get((validx * 3 + waitLine[n].first), (validy * 3 + waitLine[n].second))
                            != TA::BoardInterface::Tag::None){
                            n++;
                            std::cout << "while loop\n";
                        }
                        std::cout << "In23\n";
                    }
                }
                //cout << vai
                retx = validx * 3 + waitLine[n].first;
                rety = validy * 3 + waitLine[n].second;
                n = 0;
            }  

        }
        
        if(dbg)std::cout<<"choose:("<<retx<<","<<rety<<")\n";

        return std::make_pair(retx, rety);
    }//end query where to put

    int ultraEndnager(TA::UltraBoard& MainBoard, int i, int j, TA::BoardInterface::Tag t, int weight){
        int totalpnt=0;
        totalpnt+=enemyAround(MainBoard, i, j, t, weight);
        totalpnt+=allyAround(MainBoard, i, j, t, weight);
        return totalpnt;
    }
    int subendanger(TA::UltraBoard& MainBoard, int x, int y, TA::BoardInterface::Tag t, int weight){
        TA::Board& tgtboard = MainBoard.sub(x, y);
        int pnt = 0;
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                pnt += enemyAround(tgtboard, i, j, t, weight);
                pnt += allyAround(tgtboard, i, j, t, weight);
            }
        }
        return pnt;
    }

    bool inRange(int x, int y){
        if(x>=0 && x<3 && y>=0 && y<3) return true;
        else return false;
    }
    bool isEnemy(TA::BoardInterface& tgtBoard, int x, int y, TA::BoardInterface::Tag allytag){
        TA::BoardInterface::Tag enemytag = TA::BoardInterface::Tag::None;
        if(allytag == TA::BoardInterface::Tag::O) enemytag = TA::BoardInterface::Tag::X;
        else if(allytag == TA::BoardInterface::Tag::X) enemytag = TA::BoardInterface::Tag::O;
        if(inRange(x,y)){
            if(tgtBoard.state(x,y)==enemytag){
                return true;
            }
            else return false;
        }
        else return false;
    }
    bool isNone(TA::BoardInterface& tgtBoard, int x, int y){
        if(inRange(x,y)){
            if(tgtBoard.state(x,y)==TA::BoardInterface::Tag::None){
                return true;
            }
            else return false;
        }
        else return false;
    }

    bool isAlly(TA::BoardInterface& tgtBoard, int x, int y, TA::BoardInterface::Tag allytag){
        TA::BoardInterface::Tag enemytag;
        if(allytag == TA::BoardInterface::Tag::O) enemytag = TA::BoardInterface::Tag::X;
        else if(allytag == TA::BoardInterface::Tag::X) enemytag = TA::BoardInterface::Tag::O;
        if(inRange(x,y)){
            if(tgtBoard.state(x,y)==allytag){
                return true;
            }
            else return false;
        }
        else return false;
    }
    
    int enemyAround(TA::BoardInterface& tgtBoard, int x, int y, TA::BoardInterface::Tag t, int weight){
        int enemypnt=1*weight;
        int blockpnt=9*weight;
        int totalpnt=0;
        if(x+y==1 || x+y==3){
            //at cross, check ignore tilt
            if(y==0 && ((isEnemy(tgtBoard, x, y+1, t)&&isNone(tgtBoard, x, y+2)) || 
                (isNone(tgtBoard, x, y+1)&&isEnemy(tgtBoard, x, y+2, t)))){
                totalpnt+=enemypnt;
            }
            else if(y==0 && isEnemy(tgtBoard, x, y+1, t) && isEnemy(tgtBoard, x, y+2, t)){
                totalpnt+=blockpnt;
            }

            if(y==1 && ((isEnemy(tgtBoard, x, y-1, t)&&isNone(tgtBoard, x, y+1)) || 
                (isNone(tgtBoard, x, y-1)&&isEnemy(tgtBoard, x, y+1, t)))){
                totalpnt+=enemypnt;
            }
            else if(y==1 && isEnemy(tgtBoard, x, y-1, t) && isEnemy(tgtBoard, x, y+1, t)){
                totalpnt+=blockpnt;
            }

            if(y==2 && ((isEnemy(tgtBoard, x, y-2, t)&&isNone(tgtBoard, x, y-1)) || 
                (isNone(tgtBoard, x, y-2)&&isEnemy(tgtBoard, x, y-1, t)))){
                totalpnt+=enemypnt;
            }
            else if(y==2 && isEnemy(tgtBoard, x, y-2, t) && isEnemy(tgtBoard, x, y-1, t)){
                totalpnt+=blockpnt;
            }

            if(x==0 && ((isEnemy(tgtBoard, x+1, y, t)&&isNone(tgtBoard, x+2, y)) || 
                (isNone(tgtBoard, x+1, y)&&isEnemy(tgtBoard, x+2, y, t)))){
                totalpnt+=enemypnt;
            }
            else if(x==0 && isEnemy(tgtBoard, x+1, y, t) && isEnemy(tgtBoard, x+2, y, t)){
                totalpnt+=blockpnt;
            }

            if(x==1 && ((isEnemy(tgtBoard, x-1, y, t)&&isNone(tgtBoard, x+1, y)) || 
                (isNone(tgtBoard, x-1, y)&&isEnemy(tgtBoard, x+1, y, t)))){
                totalpnt+=enemypnt;
            }
            else if(x==1 && isEnemy(tgtBoard, x-1, y, t) && isEnemy(tgtBoard, x+1, y, t)){
                totalpnt+=blockpnt;
            }

            if(x==2 && ((isEnemy(tgtBoard, x-2, y, t)&&isNone(tgtBoard, x-1, y)) || 
                (isNone(tgtBoard, x-2, y)&&isEnemy(tgtBoard, x-1, y, t)))){
                totalpnt+=enemypnt;
            }
            else if(x==2 && isEnemy(tgtBoard, x-2, y, t) && isEnemy(tgtBoard, x-1, y, t)){
                totalpnt+=blockpnt;
            }
        }
        else if(x+y==0 || x+y==2 || x+y==4){
            //corner and middle, check all
            //cross
            if(y==0 && ((isEnemy(tgtBoard, x, y+1, t)&&isNone(tgtBoard, x, y+2)) || 
                (isNone(tgtBoard, x, y+1)&&isEnemy(tgtBoard, x, y+2, t)))){
                totalpnt+=enemypnt;
            }
            else if(y==0 && isEnemy(tgtBoard, x, y+1, t) && isEnemy(tgtBoard, x, y+2, t)){
                totalpnt+=blockpnt;
            }

            if(y==1 && ((isEnemy(tgtBoard, x, y-1, t)&&isNone(tgtBoard, x, y+1)) || 
                (isNone(tgtBoard, x, y-1)&&isEnemy(tgtBoard, x, y+1, t)))){
                totalpnt+=enemypnt;
            }
            else if(y==1 && isEnemy(tgtBoard, x, y-1, t) && isEnemy(tgtBoard, x, y+1, t)){
                totalpnt+=blockpnt;
            }

            if(y==2 && ((isEnemy(tgtBoard, x, y-2, t)&&isNone(tgtBoard, x, y-1)) || 
                (isNone(tgtBoard, x, y-2)&&isEnemy(tgtBoard, x, y-1, t)))){
                totalpnt+=enemypnt;
            }
            else if(y==2 && isEnemy(tgtBoard, x, y-2, t) && isEnemy(tgtBoard, x, y-1, t)){
                totalpnt+=blockpnt;
            }

            if(x==0 && ((isEnemy(tgtBoard, x+1, y, t)&&isNone(tgtBoard, x+2, y)) || 
                (isNone(tgtBoard, x+1, y)&&isEnemy(tgtBoard, x+2, y, t)))){
                totalpnt+=enemypnt;
            }
            else if(x==0 && isEnemy(tgtBoard, x+1, y, t) && isEnemy(tgtBoard, x+2, y, t)){
                totalpnt+=blockpnt;
            }

            if(x==1 && ((isEnemy(tgtBoard, x-1, y, t)&&isNone(tgtBoard, x+1, y)) || 
                (isNone(tgtBoard, x-1, y)&&isEnemy(tgtBoard, x+1, y, t)))){
                totalpnt+=enemypnt;
            }
            else if(x==1 && isEnemy(tgtBoard, x-1, y, t) && isEnemy(tgtBoard, x+1, y, t)){
                totalpnt+=blockpnt;
            }

            if(x==2 && ((isEnemy(tgtBoard, x-2, y, t)&&isNone(tgtBoard, x-1, y)) || 
                (isNone(tgtBoard, x-2, y)&&isEnemy(tgtBoard, x-1, y, t)))){
                totalpnt+=enemypnt;
            }
            else if(x==2 && isEnemy(tgtBoard, x-2, y, t) && isEnemy(tgtBoard, x-1, y, t)){
                totalpnt+=blockpnt;
            }
            
            //tilt
            if(!(x==1&&y==1)){
                if((isEnemy(tgtBoard, x+1, y+1, t)&&isNone(tgtBoard, x+2, y+2))||
                    (isNone(tgtBoard, x+1, y+1)&&isEnemy(tgtBoard, x+2, y+2, t))){
                    totalpnt+=enemypnt;
                }
                else if(isEnemy(tgtBoard, x+1, y+1, t)&&isEnemy(tgtBoard, x+2, y+2, t)){
                    totalpnt+=blockpnt;
                }

                if((isEnemy(tgtBoard, x+1, y-1, t)&&isNone(tgtBoard, x+2, y-2))||
                    (isNone(tgtBoard, x+1, y-1)&&isEnemy(tgtBoard, x+2, y-2, t))){
                    totalpnt+=enemypnt;
                }
                else if(isEnemy(tgtBoard, x+1, y-1, t)&&isEnemy(tgtBoard, x+2, y-2, t)){
                    totalpnt+=blockpnt;
                }

                if((isEnemy(tgtBoard, x-1, y+1, t)&&isNone(tgtBoard, x-2, y+2))||
                    (isNone(tgtBoard, x-1, y+1)&&isEnemy(tgtBoard, x-2, y+2, t))){
                    totalpnt+=enemypnt;
                }
                else if(isEnemy(tgtBoard, x-1, y+1, t)&&isEnemy(tgtBoard, x-2, y+2, t)){
                    totalpnt+=blockpnt;
                }

                if((isEnemy(tgtBoard, x-1, y-1, t)&&isNone(tgtBoard, x-2, y-2))||
                    (isNone(tgtBoard, x-1, y-1)&&isEnemy(tgtBoard, x-2, y-2, t))){
                    totalpnt+=enemypnt;
                }
                else if(isEnemy(tgtBoard, x-1, y-1, t)&&isEnemy(tgtBoard, x-2, y-2, t)){
                    totalpnt+=blockpnt;
                }
            }
            else if(x==1&&y==1){
                 if((isEnemy(tgtBoard, x+1, y+1, t)&&isNone(tgtBoard, x-1, y-1))||
                    (isNone(tgtBoard, x+1, y+1)&&isEnemy(tgtBoard, x-1, y-1, t))){
                    totalpnt+=enemypnt;
                }
                else if(isEnemy(tgtBoard, x+1, y+1, t)&&isEnemy(tgtBoard, x-1, y-1, t)){
                    totalpnt+=blockpnt;
                }

                if((isEnemy(tgtBoard, x+1, y-1, t)&&isNone(tgtBoard, x-1, y+1))||
                    (isNone(tgtBoard, x+1, y-1)&&isEnemy(tgtBoard, x-1, y+1, t))){
                    totalpnt+=enemypnt;
                }
                else if(isEnemy(tgtBoard, x+1, y-1, t)&&isEnemy(tgtBoard, x-1, y+1, t)){
                    totalpnt+=blockpnt;
                }
            }
        }
        return totalpnt;
    }
    int allyAround(TA::BoardInterface& tgtBoard, int x, int y, TA::BoardInterface::Tag t, int weight){
        //int ally=0;
        int totalpnt= 0;
        int allypnt = 2 *weight;
        int linkpnt = 12 *weight;
        int canlink = 2 *weight;

        if((x == 0 && y == 0)||(x == 0 && y == 2)||(x == 2 && y == 0)||(x == 2 && y == 2)){
            if(isAlly(tgtBoard,x,y+1,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x,y+2,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x,y+2)) totalpnt += canlink;
            } 
            if(isAlly(tgtBoard,x,y-1,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x,y-2,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x,y-2)) totalpnt += canlink;
            } 
            if(isAlly(tgtBoard,x+1,y,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x+2,y,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x+2,y)) totalpnt += canlink;
            } 
            if(isAlly(tgtBoard,x-1,y,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x-2,y,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x-2,y)) totalpnt += canlink;
            } 
            if(isAlly(tgtBoard,x+1,y+1,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x+2,y+2,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x+2,y+2)) totalpnt += canlink;
            } 
            if(isAlly(tgtBoard,x-1,y-1,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x-2,y-2,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x-2,y-2)) totalpnt += canlink;
            } 
            if(isAlly(tgtBoard,x-1,y+1,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x-2,y+2,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x-2,y+2)) totalpnt += canlink;
            } 
            if(isAlly(tgtBoard,x+1,y-1,t)){
                totalpnt += allypnt;
                if(isAlly(tgtBoard,x+2,y-2,t)) totalpnt += linkpnt;
                else if(isNone(tgtBoard,x+2,y-2)) totalpnt += canlink;
            } 
            if(isNone(tgtBoard,x,y+1) && isAlly(tgtBoard,x,y+2,t)) totalpnt += canlink;
            if(isNone(tgtBoard,x,y-1) && isAlly(tgtBoard,x,y-2,t)) totalpnt += canlink;
            if(isNone(tgtBoard,x+1,y) && isAlly(tgtBoard,x+2,y,t)) totalpnt += canlink;
            if(isNone(tgtBoard,x-1,y) && isAlly(tgtBoard,x-2,y,t)) totalpnt += canlink;
            if(isNone(tgtBoard,x+1,y+1) && isAlly(tgtBoard,x+2,y+2,t)) totalpnt += canlink;
            if(isNone(tgtBoard,x-1,y-1) && isAlly(tgtBoard,x-2,y-2,t)) totalpnt += canlink;
            if(isNone(tgtBoard,x-1,y+1) && isAlly(tgtBoard,x-2,y+2,t)) totalpnt += canlink;
            if(isNone(tgtBoard,x+1,y-1) && isAlly(tgtBoard,x+2,y-2,t)) totalpnt += canlink;
        }


        if((x == 0 && y == 1)||(x == 1 && y == 0)||(x == 1 && y == 2)||(x == 2 && y == 1)){
            if(isAlly(tgtBoard,x,y+1,t) || isAlly(tgtBoard,x, y-1,t)){
                if(isAlly(tgtBoard,x,y-1,t) && isAlly(tgtBoard,x,y+1,t)) totalpnt += (2*allypnt+ linkpnt);
                else if(isNone(tgtBoard,x,y+1) || isNone(tgtBoard,x,y-1)) totalpnt += (allypnt+canlink);
                else if(isAlly(tgtBoard,x,y+2,t) || isAlly(tgtBoard,x,y-2,t)) totalpnt += (allypnt+linkpnt);
                else if(isNone(tgtBoard,x,y+2) || isNone(tgtBoard,x,y-2)) totalpnt += (allypnt+canlink);
                else totalpnt += allypnt;
            }

            if(isAlly(tgtBoard,x-1,y,t) || isAlly(tgtBoard,x+1, y,t)){
                if(isAlly(tgtBoard,x-1,y,t) && isAlly(tgtBoard,x+1,y,t)) totalpnt += (2*allypnt+ linkpnt);
                else if(isNone(tgtBoard,x-1,y) || isNone(tgtBoard,x+1,y)) totalpnt += (allypnt+canlink);
                else if(isAlly(tgtBoard,x-2,y,t) || isAlly(tgtBoard,x+2,y,t)) totalpnt += (allypnt+linkpnt);
                else if(isNone(tgtBoard,x-2,y) || isNone(tgtBoard,x+2,y)) totalpnt += (allypnt+canlink);
                else totalpnt += allypnt;
            }

            if(isNone(tgtBoard,x-1,y) || isNone(tgtBoard,x+1,y)){
                if(isAlly(tgtBoard,x-2,y,t) || isAlly(tgtBoard,x+2,y,t)) totalpnt += canlink;
            }  
            if(isNone(tgtBoard,x,y-1) || isNone(tgtBoard,x,y+1)){
                if(isAlly(tgtBoard,x,y-2,t) || isAlly(tgtBoard,x,y+2,t)) totalpnt += canlink;
            } 
        }

        if(x == 1 && y == 1){
            if(isAlly(tgtBoard,x,y+1,t) || isAlly(tgtBoard,x, y-1,t)){
                if(isAlly(tgtBoard,x,y-1,t) && isAlly(tgtBoard,x,y+1,t)) totalpnt += (2*allypnt+ linkpnt);
                else if(isNone(tgtBoard,x,y+1) || isNone(tgtBoard,x,y-1)) totalpnt += (allypnt+canlink);
                //else if(isAlly(tgtBoard,x,y+2,t) || isAlly(tgtBoard,x,y-2,t)) totalpnt += (allypnt+linkpnt);
                //else if(isNone(tgtBoard,x,y+2) || isNone(tgtBoard,x,y-2)) totalpnt += (allypnt+canlink);
                else totalpnt += allypnt;
            }

            if(isAlly(tgtBoard,x-1,y,t) || isAlly(tgtBoard,x+1, y,t)){
                if(isAlly(tgtBoard,x-1,y,t) && isAlly(tgtBoard,x+1,y,t)) totalpnt += (2*allypnt+ linkpnt);
                else if(isNone(tgtBoard,x-1,y) || isNone(tgtBoard,x+1,y)) totalpnt += (allypnt+canlink);
                //else if(isAlly(tgtBoard,x-2,y,t) || isAlly(tgtBoard,x+2,y,t)) totalpnt += (allypnt+linkpnt);
                //else if(isNone(tgtBoard,x-2,y) || isNone(tgtBoard,x+2,y)) totalpnt += (allypnt+canlink);
                else totalpnt += allypnt;
            } 
            if(isAlly(tgtBoard,x+1,y+1,t) || isAlly(tgtBoard,x-1, y-1,t)){
                if(isAlly(tgtBoard,x-1,y-1,t) && isAlly(tgtBoard,x+1,y+1,t)) totalpnt += (2*allypnt+ linkpnt);
                else if(isNone(tgtBoard,x+1,y+1) || isNone(tgtBoard,x-1,y-1)) totalpnt += (allypnt+canlink);
                //else if(isAlly(tgtBoard,x,y+2,t) || isAlly(tgtBoard,x,y-2,t)) totalpnt += (allypnt+linkpnt);
                //else if(isNone(tgtBoard,x,y+2) || isNone(tgtBoard,x,y-2)) totalpnt += (allypnt+canlink);
                else totalpnt += allypnt;
            }

            if(isAlly(tgtBoard,x-1,y+1,t) || isAlly(tgtBoard,x+1, y-1,t)){
                if(isAlly(tgtBoard,x-1,y+1,t) && isAlly(tgtBoard,x+1,y-1,t)) totalpnt += (2*allypnt+ linkpnt);
                else if(isNone(tgtBoard,x-1,y+1) || isNone(tgtBoard,x+1,y-1)) totalpnt += (allypnt+canlink);
                //else if(isAlly(tgtBoard,x-2,y,t) || isAlly(tgtBoard,x+2,y,t)) totalpnt += (allypnt+linkpnt);
                //else if(isNone(tgtBoard,x-2,y) || isNone(tgtBoard,x+2,y)) totalpnt += (allypnt+canlink);
                else totalpnt += allypnt;
            } 
        }
        return totalpnt;
    }
    /*bool canBlock(TA::Board tgtBoard, int x, int y){
        bool ans = false;
        return ans;
    }*/
    bool canConqure(TA::Board tgtBoard, int x, int y){
        bool ans = false;

        return ans;
    }
};
