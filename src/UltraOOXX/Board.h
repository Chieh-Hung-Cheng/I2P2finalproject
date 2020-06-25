#pragma once
#include<iostream>
namespace TA {
    class BoardInterface {
    public:
        enum class Tag {
            None,
            O,
            X,
            Tie
        };
        
        friend std::ostream& operator<<(std::ostream& os, Tag& tag){
            switch(tag){
                case Tag::None:
                    os<<"None";
                    break;
                case Tag::O:
                    os<<"O";
                    break;
                case Tag::X:
                    os<<"X";
                    break;
                case Tag::Tie:
                    os<<"Tie";
                    break;
            }
            return os;
        }

        virtual Tag state(int x, int y) const = 0;

        Tag getWinTag() const {return wintag;}
        void setWinTag(Tag t) {wintag = t;}
    private:
        Tag wintag;
    };


    

    class Board : public BoardInterface {
    public:
        Board() {
            reset();
        }

        void reset() {
            setWinTag(BoardInterface::Tag::None);
            for (int i=0;i<3;++i)
                for (int j=0;j<3;++j)
                    b[i][j] = Tag::None;
        }

        bool full() const {
            for (int i=0;i<3;++i)
                for (int j=0;j<3;++j)
                    if (b[i][j] == Tag::None)
                        return false;
            return true;
        }
/*
        bool isPlaceable(int ix, int iy){
            return b[ix][iy] == Tag::None;
        }*/

        //tag of Board[ix][iy]
        Tag state(int x, int y) const override {
            return b[x][y];
        };

        Tag& get(int x, int y) {
            return b[x][y];
        }
        //TODO
        /*void set(int x, int y, Tag t){
            b[x][y] = t;
        }*/
        //end
        //Ido
       /* void determineWin(int x, int y){
            //std::cout<<"determineWin\n";
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
            if(flag &&tmp!=Tag::Tie) setWinTag(tmp); //6/6
            else{
                if(full()) setWinTag(Tag::Tie);
                else setWinTag(Tag::None);
            }
        }*/
        //I_end
        //Tag getWinTag() const {return wintag;}
        //void setWinTag(Tag t) {wintag = t;}
        
        //060505

    private:
        Tag b[3][3];
        //Tag wintag;
    };
} // Namespace TA
