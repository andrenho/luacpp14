#include "luainterface.h"

#include <cstdlib>
#include <cstdio>

#include <iostream>
using namespace std;

#include "point.h"

int main()
{
    lua::LuaInterface luax([](string s, void*) { cerr << s << endl; exit(1); }, nullptr);

    /*
    class Test {
    public:
        Test(vector<Point> const& points, Point const& position, double angle) {
            for(auto const& p: points) {
                printf("%f %f\n", p.x, p.y);
            }
        }
    };

    luax.RegisterConstructor<Test, vector<Point>, Point, double>("tes");
    luax.LoadSource("test.lua");

    luax.PushGlobal("a");
    for(auto const& p: luax.Pop<vector<Point>>()) {
        printf("%f %f\n", p.x, p.y);
    }
    */

    luax.Push(Point { 10, 200 });
    luax.StackDump();

    return 0;
}

// vim: ts=4:sw=4:sts=4:expandtab
