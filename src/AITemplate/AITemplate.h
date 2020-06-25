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
        Standard,
        Manual
    };
    friend std::ostream& operator<<(std::ostream& os, Mode& mode){
        switch(mode){
            case Mode::Manual:
                break;
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
    TA::BoardInterface::Tag enemytag;

    //for ultimate winning strategy
    std::vector<std::pair<int,int>> waitLine;
    int state;
    int n;

    //for standard mode
    int subweight=10;
    int ultraweight=2;
    int enemypnt=1;
    int blockpnt=5;
    int allypnt=1;
    int linkpnt=7;
    int canlink=1;
    int sameboardpnt=10;
    int senduselesspnt=5;
    
public:
    
    //end
    void init(bool order) override
    {
        this->x = rand()%9;
        this->y = rand()%9;
        if (order){
            this->mytag = TA::BoardInterface::Tag::O;
            this->enemytag = TA::BoardInterface::Tag::X;
            this->mode = Mode::Random;//Offense;
            this->x = 1;
            this->y = 1;
            state = -1;
            n = 0;
        }
        else {
            this->mytag = TA::BoardInterface::Tag::X;
            this->enemytag=TA::BoardInterface::Tag::O;
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
        //determineBoardWin(x, y, MainBoard, this->enemytag);
        //determineUltraWin(x, y, MainBoard, this->enemytag);

        //TA::BoardInterface::Tag oldTag = MainBoard.sub(x/3, y/3).getWinTag();
        //std::cout<<oldTag<<"?\n";


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
            while(!isVaild(retx, rety, MainBoard)){
                retx = vaildx*3 + rand()%3;
                rety = vaildy*3 + rand()%3;
            }
            
        }
        else{
            retx = rand()%9;
            rety = rand()%9;
            while(!isVaild(retx, rety, MainBoard)){
                retx = rand()%9;
                rety = rand()%9;
            }
        }
        Mode decision = Mode::Random;
        
        if(mode==Mode::Standard){
            //Standard AI mode
            int points[9][9]={0};
            int ultrapoints[9][9]={0};
            int total[9][9]={0};
            int maxpnt=-9876;


            if(confined){
                //in sub Board[vaildx][vaildy]
                //3x3 options
                for(int i=0; i<3; i++){
                    for(int j=0; j<3; j++){
                        points[i][j]=0;
                        ultrapoints[i][j]=0;
                        total[i][j]=0;
                        if(isPlaceable(i, j, tgtBoard)){
                            //empty space
                            //analyze subboard
                            if(!isOccupied(vaildx, vaildy, MainBoard)){
                                //subBoard not occupied yet
                                points[i][j]+=enemyAround(tgtBoard, i, j, this->mytag, 1);
                                points[i][j]+=allyAround(tgtBoard, i, j, this->mytag, 1);
                            
                            }
                            //else{
                                //subBoard occupied
                                //send to occupied or useless
                                if(((isOccupied(i,j, MainBoard))||canConqure(tgtBoard, i, j, this->mytag)) 
                                && (!MainBoard.sub(i,j).full())){
                                    points[i][j]+=senduselesspnt;
                                }
                            //}

                            //analyze UltraBoard
                            
                            //prevent next round still in the same subboard
                            if(vaildx==i&&vaildy==j&&(!canConqure(tgtBoard, i, j, this->mytag)))ultrapoints[i][j]+=sameboardpnt;

                            if(mytag==TA::BoardInterface::Tag::O){
                                ultrapoints[i][j]+=ultraEndnager(MainBoard, i, j, TA::BoardInterface::Tag::X, 1);
                                ultrapoints[i][j]+=subEndanger(MainBoard, i, j, TA::BoardInterface::Tag::X, 1);
                            }
                            else if(mytag==TA::BoardInterface::Tag::X){
                                ultrapoints[i][j]+=ultraEndnager(MainBoard, i, j, TA::BoardInterface::Tag::O, 1);
                                ultrapoints[i][j]+=subEndanger(MainBoard, i, j, TA::BoardInterface::Tag::O, 1);
                            }

                            //update (retx, rety) if better
                            total[i][j] = points[i][j]*subweight-ultrapoints[i][j]*ultraweight;
                            if((total[i][j]>maxpnt) || ((total[i][j]!=0&&total[i][j]==maxpnt)&&rand()%2)){
                                maxpnt = total[i][j];
                                retx = vaildx*3 + i;
                                rety = vaildy*3 + j;
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
                        total[i][j]=0;
                        if(MainBoard.get(i, j)==TA::BoardInterface::Tag::None){
                            //if placeable

                            //subboard analysis
                            if(isOccupied(i/3, j/3, MainBoard)){
                                //if subBoard not occupied yet
                                points[i][j]+=enemyAround(MainBoard.sub(i/3, j/3), i%3, j%3, this->mytag, 1);
                                points[i][j]+=allyAround(MainBoard.sub(i/3, j/3), i%3, j%3, this->mytag, 1);
                            }
                            //else{
                                if(((isOccupied(i/3,j/3, MainBoard))||canConqure(MainBoard.sub(i/3, j/3), i%3, j%3, this->mytag)) 
                                && (!MainBoard.sub(i,j).full())){
                                    points[i][j]+=senduselesspnt;
                                }
                            //}

                            //prevent same board
                            if(i/3==i%3&&j/3==j%3&&(!canConqure(MainBoard.sub(i/3,j/3), i%3, j%3, this->mytag))){
                                ultrapoints[i][j]+=sameboardpnt;
                            }

                            //ultraboard analysis
                            //bug!!!
                            if(mytag==TA::BoardInterface::Tag::O){
                                ultrapoints[i][j]+=ultraEndnager(MainBoard, i/3, j/3, TA::BoardInterface::Tag::X, 1);
                                ultrapoints[i][j]+=subEndanger(MainBoard, i/3, j/3, TA::BoardInterface::Tag::X, 1);
                            }
                            else if(mytag==TA::BoardInterface::Tag::X){
                                ultrapoints[i][j]+=ultraEndnager(MainBoard, i/3, j/3, TA::BoardInterface::Tag::O, 1);
                                ultrapoints[i][j]+=subEndanger(MainBoard, i/3, j/3, TA::BoardInterface::Tag::O, 1);
                            }

                            //update if better
                            total[i][j] = points[i][j]*subweight-ultrapoints[i][j]*ultraweight;
                            if((total[i][j]>maxpnt) || ((total[i][j]!=0&&total[i][j]==maxpnt)&&rand()%2)){
                                maxpnt = total[i][j];
                                retx = i;
                                rety = j;
                                decision = Mode::Standard;
                            }
                        }
                    }
                }
            }

            //print point for debug
            if(0){//dbg
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
                std::cout<<std::endl;
                //UltraBoard data
                for(int i=0; i<9; i++){
                    if(i==0)std::cout<<" ";
                    if(confined){
                        std::cout<<" "<<vaildy*3+i;
                        if(i==2)break;
                    }
                    else std::cout<<" "<<i;
                }
                std::cout<<std::endl;
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
                std::cout<<std::endl;
                //total data
                for(int i=0; i<9; i++){
                    if(i==0)std::cout<<" ";
                    if(confined){
                        std::cout<<" "<<vaildy*3+i;
                        if(i==2)break;
                    }
                    else std::cout<<" "<<i;
                }
                std::cout<<std::endl;
                for(int i=0; i<9; i++){
                    if(confined)std::cout<<vaildx*3+i;
                    else std::cout<<i; 

                    for(int j=0; j<9; j++){
                        if(confined){
                            std::cout<<" "<<total[i][j];
                            if(j==2)break;
                        }
                        else std::cout<<" "<<total[i][j];
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
        else if(mode==Mode::Manual){
            //confined by 1000ms time limit
            std::cout<<"Your turn! Where to put?(enter x y):"<<std::endl;
            std::cin>>retx>>rety;
        }
        
        //determineBoardWin(retx, rety, MainBoard, this->mytag);//not working
        //determineUltraWin(retx, rety, MainBoard, this->mytag);
        
        //TA::BoardInterface::Tag newTag = tgtBoard.getWinTag();
        //std::cout << newTag << "??\n";
        if(dbg)std::cout<<"choose:("<<retx<<","<<rety<<")\n";


        return std::make_pair(retx, rety);
    }//end query where to put

    int ultraEndnager(TA::UltraBoard& MainBoard, int i, int j, TA::BoardInterface::Tag t, int weight){
        int totalpnt=0;
        totalpnt+=enemyAround(MainBoard, i, j, t, weight);
        totalpnt+=allyAround(MainBoard, i, j, t, weight);
        return totalpnt;
    }
    int subEndanger(TA::UltraBoard& MainBoard, int x, int y, TA::BoardInterface::Tag t, int weight){
        TA::Board& tgtboard = MainBoard.sub(x, y);
        int pnt = 0;

        if(tgtboard.getWinTag()==TA::BoardInterface::Tag::None){
            //if subBoard not occupied yet
            for(int i = 0; i < 3; i++){
                for(int j = 0; j < 3; j++){
                    pnt += enemyAround(tgtboard, i, j, t, weight);
                    pnt += allyAround(tgtboard, i, j, t, weight);
                }
            }
        }
        else if(tgtboard.getWinTag()!=TA::BoardInterface::Tag::None){
            for(int i=0; i<3; i++){
                for(int j=0; j<3; j++){
                    if(isOccupied(i,j, MainBoard)&&(!MainBoard.sub(i,j).full())){
                        pnt+=senduselesspnt;
                    }
                }
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
        //TA::BoardInterface::Tag enemytag;
        //if(allytag == TA::BoardInterface::Tag::O) enemytag = TA::BoardInterface::Tag::X;
        //else if(allytag == TA::BoardInterface::Tag::X) enemytag = TA::BoardInterface::Tag::O;
        if(inRange(x,y)){
            if(tgtBoard.state(x,y)==allytag){
                return true;
            }
            else return false;
        }
        else return false;
    }
    
    int enemyAround(TA::BoardInterface& tgtBoard, int x, int y, TA::BoardInterface::Tag t, int weight){
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
        return totalpnt*weight;
    }
    int allyAround(TA::BoardInterface& tgtBoard, int x, int y, TA::BoardInterface::Tag t, int weight){
        //int ally=0;
        int totalpnt= 0;

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
        return totalpnt*weight;
    }
    /*bool canBlock(TA::Board tgtBoard, int x, int y){
        bool ans = false;
        return ans;
    }*/
    bool canConqure(TA::Board tgtBoard, int x, int y, TA::BoardInterface::Tag t){
        if(y==0 && isAlly(tgtBoard, x, y+1, t) && isAlly(tgtBoard, x, y+2, t))return true;
        if(y==1 && isAlly(tgtBoard, x, y-1, t) && isAlly(tgtBoard, x, y+1, t))return true;
        if(y==2 && isAlly(tgtBoard, x, y-2, t) && isAlly(tgtBoard, x, y-1, t))return true;
        if(x==0 && isAlly(tgtBoard, x+1, y, t) && isAlly(tgtBoard, x+2, y, t))return true;
        if(x==1 && isAlly(tgtBoard, x-1, y, t) && isAlly(tgtBoard, x+1, y, t))return true;
        if(x==2 && isAlly(tgtBoard, x-2, y, t) && isAlly(tgtBoard, x-1, y, t))return true;

        if(x+y==0 || x+y==2 || x+y==4){
            //tilt
            if(!(x==1&&y==1)){
                if(isAlly(tgtBoard, x+1, y+1, t)&&isAlly(tgtBoard, x+2, y+2, t))return true;
                if(isAlly(tgtBoard, x+1, y-1, t)&&isAlly(tgtBoard, x+2, y-2, t))return true;
                if(isAlly(tgtBoard, x-1, y+1, t)&&isAlly(tgtBoard, x-2, y+2, t))return true;
                if(isAlly(tgtBoard, x-1, y-1, t)&&isAlly(tgtBoard, x-2, y-2, t))return true;
            }
            else if(x==1&&y==1){
                if(isAlly(tgtBoard, x+1, y+1, t)&&isAlly(tgtBoard, x-1, y-1, t))return true;
                if(isAlly(tgtBoard, x+1, y-1, t)&&isAlly(tgtBoard, x-1, y+1, t))return true;
            }
        }

        return false;
    }
    /*void determineUltraWin(int x, int y, TA::UltraBoard &MainBoard, TA::BoardInterface::Tag t){
        //std::cout<<"determineWin\n";
        x /= 3;
        y /= 3;
        if(MainBoard.getWinTag() != TA::BoardInterface::Tag::None) return;
        TA::BoardInterface::Tag tmp = t;
        int flag = 0;
        if(y == 0 && tmp == MainBoard.state(x, y+1) && tmp == MainBoard.state(x, y+2)) flag = 1;
        if(y == 1 && tmp == MainBoard.state(x, y-1) && tmp == MainBoard.state(x, y+1)) flag = 1;
        if(y == 2 && tmp == MainBoard.state(x, y-1) && tmp == MainBoard.state(x, y-2)) flag = 1;
        if(x == 0 && tmp == MainBoard.state(x+1, y) && tmp == MainBoard.state(x+2, y)) flag = 1;
        if(x == 1 && tmp == MainBoard.state(x-1, y) && tmp == MainBoard.state(x+1, y)) flag = 1;
        if(x == 2 && tmp == MainBoard.state(x-1, y) && tmp == MainBoard.state(x-2, y)) flag = 1;
        if((tmp == MainBoard.state(0, 0) && tmp == MainBoard.state(1, 1) && tmp == MainBoard.state(2, 2)) || 
            (tmp == MainBoard.state(0, 2) && tmp == MainBoard.state(1, 1) && tmp == MainBoard.state(2, 0))) flag = 1;
        if(flag &&tmp!=TA::BoardInterface::Tag::Tie) MainBoard.setWinTag(tmp); //6/6
        else{
            if(MainBoardfull(MainBoard)) MainBoard.setWinTag(TA::BoardInterface::Tag::Tie);
            else MainBoard.setWinTag(TA::BoardInterface::Tag::None);
        }
        //TA::BoardInterface::Tag temp = MainBoard.getWinTag();
        //std::cout<<temp<<"\n";
    }

    void determineBoardWin(int x, int y, TA::UltraBoard &MainBoard, TA::BoardInterface::Tag t){
        TA::Board& tgtboard = MainBoard.sub(x/3, y/3);
        if(tgtboard.getWinTag() != TA::BoardInterface::Tag::None) return;
        std::cout<<"determineWin"<<x<<y<<"\n";

        x %= 3;
        y %= 3;
        TA::BoardInterface::Tag tmp = t;
        //std::cout<<tmp<<"\n";
        int flag = 0;
        if(y == 0 && tmp == tgtboard.state(x, y+1) && tmp == tgtboard.state(x, y+2)) flag = 1;
        if(y == 1 && tmp == tgtboard.state(x, y-1) && tmp == tgtboard.state(x, y+1)) flag = 1;
        if(y == 2 && tmp == tgtboard.state(x, y-1) && tmp == tgtboard.state(x, y-2)) flag = 1;
        if(x == 0 && tmp == tgtboard.state(x+1, y) && tmp == tgtboard.state(x+2, y)) flag = 1;
        if(x == 1 && tmp == tgtboard.state(x-1, y) && tmp == tgtboard.state(x+1, y)) flag = 1;
        if(x == 2 && tmp == tgtboard.state(x-1, y) && tmp == tgtboard.state(x-2, y)) flag = 1;
        if(((x == 0 && y == 0) && tmp == tgtboard.state(1, 1) && tmp == tgtboard.state(2, 2)) || 
           ((x == 1 && y == 1) && tmp == tgtboard.state(0, 0) && tmp == tgtboard.state(2, 2)) ||
           ((x == 2 && y == 2) && tmp == tgtboard.state(0, 0) && tmp == tgtboard.state(1, 1)) ||
           ((x == 0 && y == 2) && tmp == tgtboard.state(1, 1) && tmp == tgtboard.state(2, 0)) ||
           ((x == 1 && y == 1) && tmp == tgtboard.state(0, 2) && tmp == tgtboard.state(2, 0)) ||
           ((x == 2 && y == 0) && tmp == tgtboard.state(0, 2) && tmp == tgtboard.state(1, 1))) flag = 1;
        if(flag &&tmp!=TA::BoardInterface::Tag::Tie){
             tgtboard.setWinTag(tmp);
             //std::cout<<"tag change "<<t<<"\n";
        }
        else{
            //std::cout<<"tag no change "<<t<<"\n";
            if(tgtboard.full()) tgtboard.setWinTag(TA::BoardInterface::Tag::Tie);
            else tgtboard.setWinTag(TA::BoardInterface::Tag::None);
        }
        TA::BoardInterface::Tag temp= tgtboard.getWinTag();
        //std::cout<<temp<<"!\n";
    }
    bool MainBoardfull(TA::UltraBoard board) const {
        for (int i=0;i<3;++i)
            for (int j=0;j<3;++j)
                if (!board.sub(i, j).full())
                    return false;
        return true;
    }*/

        bool isOccupied(int ix, int iy, TA::UltraBoard& MainBoard){
        return !(MainBoard.sub(ix, iy).getWinTag()==TA::BoardInterface::Tag::None);
    }

    bool isVaild(int x, int y, TA::UltraBoard& MainBoard) const{
        if(MainBoard.sub(x/3, y/3).state(x%3, y%3) == TA::BoardInterface::Tag::None)return true;
        else return false;
    }

    bool isPlaceable(int ix, int iy, TA::BoardInterface &board){
        return board.state(ix,iy) == TA::BoardInterface::Tag::None;
    }
};
