#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>

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
    
public:
    
    //end
    void init(bool order) override
    {
        this->x = rand()%9;
        this->y = rand()%9;
        if(order)this->mytag = TA::BoardInterface::Tag::O;
        else this->mytag = TA::BoardInterface::Tag::X;
        this->mode = Mode::Standard;
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
            int points[9][9]={0};
            int maxpnt=0;
            if(confined){
                //in sub Board[vaildx][vaildy]
                //3x3 options
                if(!MainBoard.isOccupied(vaildx, vaildy)){
                    //subBoard not occupied yet
                    for(int i=0; i<3; i++){
                        for(int j=0; j<3; j++){
                            if(tgtBoard.isPlaceable(i, j)){
                                //empty space
                                points[i][j]+=enemyAround(tgtBoard, i, j, this->mytag, 1);
                                int allypnt = allyAround(tgtBoard, i, j, this->mytag);
                                std::cout<<"test ally:"<<i<<j<<' '<<allypnt<<std::endl;
                                points[i][j]+=allypnt;
                                //update (retx, rety) if better
                                if(points[i][j]>maxpnt || ((points[i][j]!=0&&points[i][j]==maxpnt)&&rand()%2)){
                                    maxpnt = points[i][j];
                                    retx = vaildx*3+i;
                                    rety = vaildy*3+j;
                                    decision = Mode::Standard;
                                }
                            }
                        }
                    }
                }
                else{
                    //subBoard occupied or tied(impossible, full)
                    for(int i=0; i<3; i++){
                        for(int j=0; j<3; j++){
                            if(tgtBoard.isPlaceable(i, j)){
                                if((!MainBoard.isOccupied(i,j)) && (!MainBoard.sub(i,j).full())){
                                    points[i][j]+=3;
                                }
                                
                                if(mytag==TA::BoardInterface::Tag::O){
                                    points[i][j]+=enemyAround(MainBoard, i/3, j/3, TA::BoardInterface::Tag::X, 10);
                                }
                                else if(mytag==TA::BoardInterface::Tag::X){
                                    points[i][j]+=enemyAround(MainBoard, i/3, j/3, TA::BoardInterface::Tag::O, 10);
                                }

                                if(points[i][j]>maxpnt || ((points[i][j]!=0&&points[i][j]==maxpnt)&&rand()%2)){
                                    maxpnt = points[i][j];
                                    retx = vaildx*3+i;
                                    rety = vaildy*3+j;
                                    decision = Mode::Standard;
                                }
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
                        if(MainBoard.get(i, j)==TA::BoardInterface::Tag::None){
                            //if placeable
                            if(MainBoard.state(i/3, j/3)==TA::BoardInterface::Tag::None){
                                //if subBoard not occupied yet
                                points[i][j]+=enemyAround(MainBoard.sub(i/3, j/3), i%3, j%3, this->mytag, 1);
                                points[i][j]+=allyAround(MainBoard.sub(i/3, j/3), i%3, j%3, this->mytag);
                            }

                            //update if better
                            if(points[i][j]>maxpnt){
                                maxpnt = points[i][j];
                                retx = i;
                                rety = j;
                                decision = Mode::Standard;
                            }
                        }
                    }
                }
            }

            //print point for debug
            //some flush problem
            if(dbg){
                std::cout<<"Decision by MODE::"<<decision<<std::endl;
                std::cout<<"Points analysis:\n";
                
                
                //std::cout<<"  "<<vaildy*3<<" "<<vaildy*3+1<<" "<<vaildy*3+2<<std::endl;
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
                            std::cout<<" "<<points[i][j];
                            if(j==2)break;
                        }
                        else std::cout<<" "<<points[i][j];
                    }
                    std::cout<<"\n";
                    if(confined&&i==2)break;
                }
            }
        }
        else if(mode==Mode::Offense){
            //Ultimate Winning Strategy Here
        }
        
        if(dbg)std::cout<<"choose:("<<retx<<","<<rety<<")\n";

        return std::make_pair(retx, rety);
    }//end query where to put

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

    bool isAlly(TA::Board tgtBoard, int x, int y, TA::BoardInterface::Tag allytag){
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
        //X None X is not considered!
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
    int allyAround(TA::Board tgtBoard, int x, int y, TA::BoardInterface::Tag t){
        //int ally=0;
        int totalpnt= 0;
        int allypnt = 2;
        int linkpnt = 12;
        int canlink = 3;

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
