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

// class State must have
//   + Lexer
//     - void setPos(auto)
//     - auto getPos()
//   + Cache
//     - Any& cache()
    
template<typename State>
using Parser = std::function<RCode (State&, bool)>;
    
template<typename State>
using Actor = std::function<void (State&)>;
    
// FUNCTIONS
template<typename State>
Parser<State> normalize(Parser<State> parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            auto pos = state.getPos();
            RCode rc = parser(state, must);
            if(RCode::SUCCESS != rc)
            {
                state.setPos(pos);
            }
            return rc;
        };
}

template<typename State>
Parser<State> action(Actor<State> actor)
{
    return
        [actor](State& state, bool must)->RCode
        {
            auto pos = state.getPos();
            actor(state);
            state.setPos(pos);
            return RCode::IGNORE;
        };
}

template<typename State, typename CacheType, typename Ans>
Parser<State> capture(Ans& ans)
{
    return
        [&ans](State& state, bool must)->RCode
        {
            ans = state.cache().template get<CacheType>();
            return yapeg::RCode::IGNORE;
        };
}

template<typename State, typename Ans>
RCode invoke(Parser<State> parser, State& state, bool must, Ans& ans)
{
    RCode rc = parser(state, must);
    if(RCode::FAIL != rc)
    {
        state.cache().set(ans);
    }
    return rc;
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
Parser<State> combo(Parser<State> parser1, Parser<State> parser2)
{
    return seq<State>({parser1, parser2});
}

template<typename State>
Parser<State> combo(Parser<State> parser, Actor<State> actor)
{
    return combo<State>(parser, action(actor));
}
    
template<typename State>
Parser<State> choice(const std::vector< Parser<State> >& parsers)
{
    return
        [parsers](State& state, bool must)->RCode
        {
            for(auto it = parsers.begin(); it != parsers.end(); ++it)
            {
                if(RCode::SUCCESS ==
                   (*it)(state, it+1 != parsers.end() ? false : must))
                {
                    return RCode::SUCCESS;
                }
            }
            return RCode::FAIL;
        };
}
    
template<typename State>
Parser<State> choice(const std::vector< Parser<State> >& parsers, Parser<State> action)
{
    return seq<State>({choice<State>(parsers), action});
}

template<typename State>
Parser<State> choice(const std::vector< Parser<State> >& parsers, Actor<State> actor)
{
    return choice(parsers, action(actor));
}
    
template<typename State>
Parser<State> star(Parser<State> parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            while(RCode::FAIL != parser(state, false)) ;
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
            parser(state, false);
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
