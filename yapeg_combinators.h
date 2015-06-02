#ifndef INCLUDED_YAPEG_COMBINATORS_H
#define INCLUDED_YAPEG_COMBINATORS_H

#include <functional>
#include <initializer_list>
#include <vector>

namespace yapeg {

template<typename State>
struct Combinators
{
    
// TYPES
enum class RCode {
    SUCCESS
  , FAIL
};

// class State must have
//   + Lexer
//     - void setPos(auto)
//     - auto getPos()
//   + Cache
//     - Any& cache()
    
using Parser = std::function<RCode (State&, bool)>;
using Actor = std::function<void (State&)>;
    
// FUNCTIONS
static Parser normalize(Parser parser)
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

static Parser action(Actor actor, RCode rc)
{
    return
        [actor, rc](State& state, bool must)->RCode
        {
            auto pos = state.getPos();
            actor(state);
            state.setPos(pos);
            return rc;
        };
}

static Parser yaction(Actor actor)
{
    return action(actor, RCode::SUCCESS);
}

static Parser naction(Actor actor)
{
    return action(actor, RCode::FAIL);
}

template<typename Ans>
static RCode invoke(Parser parser, State& state, bool must, Ans& ans)
{
    RCode rc = parser(state, must);
    if(RCode::FAIL != rc)
    {
        state.cache().set(ans);
    }
    return rc;
}

template<typename CacheType, typename Ans>
static Actor capture(Ans& ans)
{
    return
        [&ans](State& state)
        {
            ans = state.cache().template get<CacheType>();
        };
}
    
static Parser seq(const std::vector<Parser>& parsers)
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

static Parser combo(Parser parser, Actor actor)
{
    return seq({parser, yaction(actor)});
}
    
static Parser choice(const std::vector<Parser>& parsers)
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

static Parser choice(const std::vector<Parser>& parsers, Actor actor)
{
    return seq({ choice(parsers), yaction(actor) });
}
    
static Parser star(Parser parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            while(RCode::FAIL != parser(state, false)) ;
            return RCode::SUCCESS;
        };
}

static Parser plus(Parser parser)
{
    return seq({parser, star(parser)});
}

static Parser qmark(Parser parser)
{
    return
        [parser](State& state, bool must)->RCode
        {
            parser(state, false);
            return RCode::SUCCESS;
        };
}

static Parser ptest(Parser parser)
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

static Parser ntest(Parser parser)
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

}; // close struct Combinators
    
} // close namespace yapeg

#endif // INCLUDED_YAPEG_COMBINATORS_H
