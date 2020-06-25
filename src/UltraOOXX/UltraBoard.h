#pragma once

#include <UltraOOXX/Board.h>
#include <iostream>
namespace TA {
    class UltraBoard : public BoardInterface {
    public:
        UltraBoard() {
            reset();
        }

        void reset() {
            for (int i=0;i<3;++i)
                for (int j=0;j<3;++j)
                    b[i][j].reset();
            //060504
            //this->wintag = Tag::None;
        }

        //get UltraBoard[gx][gy]
        Board::Tag& get(int x, int y) {
            return b[x/3][y/3].get(x%3, y%3);
        }

        Board& sub(int ix, int iy) {
            return b[ix][iy];
        }

        //tag of UltraBoard[ix][iy]
        Tag state(int x, int y) const override {
            return b[x][y].getWinTag();
        };
        //TODO
        /*bool full() const {
            for (int i=0;i<3;++i)
                for (int j=0;j<3;++j)
                    if (!b[i][j].full())
                        return false;
            return true;
        }

        bool isOccupied(int ix, int iy){
            return !(b[ix][iy].getWinTag()==Tag::None);
        }

        bool isVaild(int x, int y) const{
            if(b[x/3][y/3].state(x%3, y%3) == Tag::None)return true;
            else return false;
        }

        void set(int x, int y, Tag t){
            b[x/3][y/3].set(x%3, y%3, t);
        }*/
        //end
        //Ido
        /*void determineWin(int x, int y){
            //std::cout<<"determine_U_Win\n";
            if(getWinTag() != Tag::None) return;
            Tag tmp = state(x,y);
            int flag = 0;
            if(y == 0 && tmp == state(x, y+1) && tmp == state(x, y+2)) flag = 1;
            if(y == 1 && tmp == state(x, y-1) && tmp == state(x, y+1)) flag = 1;
            if(y == 2 && tmp == state(x, y-1) && tmp == state(x, y-2)) flag = 1;
            if(x == 0 && tmp == state(x+1, y) && tmp == state(x+2, y)) flag = 1;
            if(x == 1 && tmp == state(x-1, y) && tmp == state(x+1, y)) flag = 1;
            if(x == 2 && tmp == state(x-1, y) && tmp == state(x-2, y)) flag = 1;
            if((tmp == state(0, 0) && tmp == state(1, 1) && tmp == state(2, 2)) || 
               (tmp == state(0, 2) && tmp == state(1, 1) && tmp == state(2, 0))) flag = 1;
            if(flag &&tmp!=Tag::Tie) setWinTag(tmp);
            else{
                if(full()) setWinTag(Tag::Tie);
                else setWinTag(Tag::None);
            }
        }*/
        //I_end

        //060504
        //Tag getWinTag() const {return wintag;}
        //void setWinTag(Tag t) {wintag = t;}

    private:
        Board b[3][3];
        //060504
        //Tag wintag;
    };
} // Namespace TA
