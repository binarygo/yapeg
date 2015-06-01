#include <gtest/gtest.h>
#include <yapeg_combinators.h>
#include <yapeg_any.h>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <cassert>
#include <iostream>

namespace yapeg {

namespace {

using Token = std::pair<std::string, std::string>;
std::ostream& operator<< (std::ostream& out, const Token& rhs)
{
    out << "(" << rhs.first << ",'" << rhs.second << "')";
    return out;
}    
    
class State
{
private:
    // DATA
    std::size_t d_pos;
    std::vector<Token> d_tokens;
    Any d_cache;
    
public:
    // CREATORS
    State(std::initializer_list<Token> tokens)
        : d_pos(0)
        , d_tokens(tokens) {}

    // MANIPULATORS
    void next()
    {
        ++d_pos;
    }

    void setPos(std::size_t pos)
    {
        assert(pos <= d_tokens.size());
        d_pos = pos;
    }

    Any& cache() { return d_cache; }
    
    // ACCESSORS
    bool isValid() const
    {
        return d_pos < d_tokens.size();
    }
    const Token& token() const
    {
        assert(d_pos < d_tokens.size());
        return d_tokens[d_pos];
    }
    
    std::size_t getPos() const
    {
        return d_pos;
    }

    const Any& cache() const { return d_cache; }
    
    std::size_t size() const
    {
        return d_tokens.size();
    }

    const std::vector<Token>& tokens() const
    {
        return d_tokens;
    }
};    

Parser<State> baseParser(const std::string& tokenType)
{
    auto p =
        [tokenType](State& s, bool must)->RCode
        {
            if(s.isValid() && s.token().first == tokenType)
            {
                s.cache().set(Token(s.token()));
                s.next();
                return RCode::SUCCESS;
            }
            if(must)
            {
                throw std::runtime_error(
                    "unexpected token, expect " + tokenType);
            }
            return RCode::FAIL;
        };
    return normalize<State>(p);
}

RCode dummyParser(State& s, bool must)
{
    return RCode::SUCCESS;
}
    
} // close anonymous namespace

TEST(Combinators, test)
{
    State state({ Token("int", "123"), Token("float", "99.9") });

    RCode rc = seq<State>({dummyParser, dummyParser})(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 0u);
}
    
TEST(Combinators, seq)
{
    State state({ Token("int", "123"), Token("float", "99.9") });

    RCode rc =
        seq<State>({
            baseParser("int"),
            yaction<State>(
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            baseParser("float"),
            yaction<State>(
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 2u);
}

TEST(Combinators, choice)
{
    State state({
        Token("int", "123"),
        Token("double", "6.18"),
        Token("string", "hello")
    });

    RCode rc =
        seq<State>({
            baseParser("int"),
            yaction<State>(
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            choice<State>({
                combo<State>(
                    baseParser("double"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                    }
                ),
                combo<State>(
                    baseParser("float"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_TRUE(0);
                    }
                ),
                combo<State>(
                    baseParser("string"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_TRUE(0);
                    }
                ),
            }),            
            choice<State>({
                combo<State>(
                    baseParser("double"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_TRUE(0);
                    }
                ),
                combo<State>(
                    baseParser("float"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_TRUE(0);
                    }
                ),
                combo<State>(
                    baseParser("string"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[2]);
                    }
                ),
            })
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 3u);
}

TEST(Combinators, star1)
{
    State state({
        Token("double", "6.18"),
        Token("int", "123"),
        Token("int", "456"),
        Token("int", "789"),
        Token("string", "hello")
    });

    std::size_t i = 1;
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            star<State>({
                combo<State>(
                    baseParser("int"),
                    [&i](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[i++]);
                    }
                )
            }),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[4]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 5u);
    EXPECT_EQ(i, 4u);
}

TEST(Combinators, star2)
{
    State state({
        Token("double", "6.18"),
        Token("string", "hello")
    });

    std::size_t i = 1;
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            star<State>({
                combo<State>(
                    baseParser("int"),
                    [&i](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[i++]);
                    }
                )
            }),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 2u);
    EXPECT_EQ(i, 1u);
}

TEST(Combinators, plus1)
{
    State state({
        Token("double", "6.18"),
        Token("int", "123"),
        Token("int", "456"),
        Token("int", "789"),
        Token("string", "hello")
    });

    std::size_t i = 1;
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            plus<State>({
                combo<State>(
                    baseParser("int"),
                    [&i](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[i++]);
                    }
                )
            }),
            combo<State>(
                baseParser("string"),
                [&i](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[i]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 5u);
    EXPECT_EQ(i, 4u);
}

TEST(Combinators, plus2)
{
    State state({
        Token("double", "6.18"),
        Token("int", "123"),
        Token("string", "hello")
    });

    std::size_t i = 1;
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            plus<State>({
                combo<State>(
                    baseParser("int"),
                    [&i](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[i++]);
                    }
                )
            }),
            combo<State>(
                baseParser("string"),
                [&i](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[i]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 3u);
    EXPECT_EQ(i, 2u);
}
    
TEST(Combinators, plus3)
{
    State state({
        Token("int", "10"),
        Token("double", "6.18"),
        Token("string", "hello")
    });

    state.setPos(1);
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                }
            ),
            plus<State>({
                combo<State>(
                    baseParser("int"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_TRUE(0);
                    }
                )
            }),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_TRUE(0);
                }
            )
        })(state, false);

    EXPECT_EQ(rc, RCode::FAIL);
    EXPECT_EQ(state.getPos(), 1u);
}

TEST(Combinators, qmark1)
{
    State state({
        Token("double", "6.18"),
        Token("int", "123"),
        Token("string", "hello")
    });
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            qmark<State>({
                combo<State>(
                    baseParser("int"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                    }
                )
            }),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[2]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 3u);
}
    
TEST(Combinators, qmark2)
{
    State state({
        Token("int", "10"),
        Token("double", "6.18"),
        Token("string", "hello")
    });

    state.setPos(1);
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                }
            ),
            qmark<State>({
                combo<State>(
                    baseParser("int"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_TRUE(0);
                    }
                )
            }),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[2]);
                }
            )
        })(state, false);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 3u);
}

TEST(Combinators, ptest1)
{
    State state({
        Token("double", "6.18"),
        Token("int", "123"),
        Token("string", "hello")
    });
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            ptest<State>({
                combo<State>(
                    baseParser("int"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                    }
                )
            }),
            combo<State>(
                baseParser("int"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                }
            ),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[2]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 3u);
}
    
TEST(Combinators, ptest2)
{
    State state({
        Token("double", "6.18"),
        Token("float", "123"),
        Token("string", "hello")
    });
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            ptest<State>({
                combo<State>(
                    baseParser("int"),
                    [](State& s) {
                        EXPECT_TRUE(0);
                    }
                )
            }),
            combo<State>(
                baseParser("float"),
                [](State& s) {
                    EXPECT_TRUE(0);
                }
            ),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    EXPECT_TRUE(0);
                }
            )
        })(state, false);

    EXPECT_EQ(rc, RCode::FAIL);
    EXPECT_EQ(state.getPos(), 0u);
}

TEST(Combinators, ntest1)
{
    State state({
        Token("double", "6.18"),
        Token("int", "123"),
        Token("string", "hello")
    });
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            ntest<State>({
                combo<State>(
                    baseParser("float"),
                    [](State& s) {
                        EXPECT_TRUE(0);
                    }
                )
            }),
            combo<State>(
                baseParser("int"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                }
            ),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[2]);
                }
            )
        })(state, true);

    EXPECT_EQ(rc, RCode::SUCCESS);
    EXPECT_EQ(state.getPos(), 3u);
}
    
TEST(Combinators, ntest2)
{
    State state({
        Token("double", "6.18"),
        Token("float", "123"),
        Token("string", "hello")
    });
    
    RCode rc =
        seq<State>({
            combo<State>(
                baseParser("double"),
                [](State& s) {
                    std::cout << s.cache().get<Token>() << std::endl;
                    EXPECT_EQ(s.cache().get<Token>(), s.tokens()[0]);
                }
            ),
            ntest<State>({
                combo<State>(
                    baseParser("float"),
                    [](State& s) {
                        std::cout << s.cache().get<Token>() << std::endl;
                        EXPECT_EQ(s.cache().get<Token>(), s.tokens()[1]);
                    }
                )
            }),
            combo<State>(
                baseParser("float"),
                [](State& s) {
                    EXPECT_TRUE(0);
                }
            ),
            combo<State>(
                baseParser("string"),
                [](State& s) {
                    EXPECT_TRUE(0);
                }
            )
        })(state, false);

    EXPECT_EQ(rc, RCode::FAIL);
    EXPECT_EQ(state.getPos(), 0u);
}
    
} // close namespace yapeg

