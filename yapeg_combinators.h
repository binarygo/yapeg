#ifndef INCLUDED_YAPEG_COMBINATORS_H
#define INCLUDED_YAPEG_COMBINATORS_H

#include <functional>
#include <initializer_list>
#include <vector>

namespace yapeg {

// TYPES
enum class RCode {
    SUCCESS
  , FAIL
  , IGNORE
};

template<typename State>
using Parser = std::function<RCode (State&, bool)>;
// class State must have
//   + Lexer
//     - void setPos(auto)
//     - auto getPos()
    
template<typename State>
using Actor = std::function<void (State&)>;

// FUNCTIONS
template<typename State>
Parser<State> action(Actor<State> actor)
{
    return
        [actor](State& state, bool must)->RCode
        {
            actor(state);
            return RCode::IGNORE;
        };
}

template<typename State>
Parser<State> combo(Parser<State> parser, Actor<State> actor)
{
    return
        [parser, actor](State& state, bool must)->RCode
        {
            RCode rc = parser(state, must);
            if(RCode::FAIL != rc) actor(state);
            return rc;
        };
}

template<typename State>
Parser<State> seq(const std::vector< Parser<State> >& parsers)
{
    return
        [parsers](State& state, bool must)->RCode
        {
            auto pos = state.getPos();
            for(auto it = parsers.begin(); it != parsers.end(); ++it)
            {
                if(RCode::FAIL == (*it)(state, must))
                {
                    state.setPos(pos);
                    return RCode::FAIL;
                }
            }
            return RCode::SUCCESS;
        };
}

template<typename State>
Parser<State> choice(const std::vector< Parser<State> >& parsers)
{
    return
        [parsers](State& state, bool must)->RCode
        {
            for(auto it = parsers.begin(); it != parsers.end(); ++it)
            {
                auto pos = state.getPos();
                if(RCode::SUCCESS ==
                   (*it)(state, it+1 != parsers.end() ? false : must))
                {
                    return RCode::SUCCESS;
                }
                else
                {
                    state.setPos(pos);
                }
            }
            return RCode::FAIL;
        };
}

template<typename State>
Parser<State> star(Parser<State> parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            while(true)
            {
                auto pos = state.getPos();
                if(RCode::FAIL == parser(state, false))
                {
                    state.setPos(pos);
                    break;
                }
            }
            return RCode::SUCCESS;
        };
}

template<typename State>
Parser<State> plus(Parser<State> parser)
{
    return seq<State>({parser, star<State>(parser)});
}

template<typename State>
Parser<State> qmark(Parser<State> parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            auto pos = state.getPos();
            if(RCode::FAIL == parser(state, false))
            {
                state.setPos(pos);
            }
            return RCode::SUCCESS;
        };
}

template<typename State>
Parser<State> ptest(Parser<State> parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            auto pos = state.getPos();
            RCode rc = parser(state, false);
            state.setPos(pos);
            return
                RCode::FAIL == rc ?
                RCode::FAIL : RCode::SUCCESS;
        };
}

template<typename State>
Parser<State> ntest(Parser<State> parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            auto pos = state.getPos();
            RCode rc = parser(state, false);
            state.setPos(pos);
            return
                RCode::SUCCESS == rc ?
                RCode::FAIL : RCode::SUCCESS;
        };
}
 
} // close namespace yapeg

#endif // INCLUDED_YAPEG_COMBINATORS_H
