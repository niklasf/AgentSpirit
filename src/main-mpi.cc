#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/nonblocking.hpp>

#include "term.h"
#include "printer.h"
#include "serialization.h"
#include "runtime/logical_consequence.h"
#include "runtime/environment.h"
#include "runtime/interpreter.h"
#include "grammar/comment_skipper.h"
#include "grammar/agent_parser.h"

namespace mpi = boost::mpi;

namespace {

struct Message {
    as::Term term;

    std::string recipient;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & term;
        ar & recipient;
    }
};

class MpiEnvironment : public as::runtime::Environment {
public:
    MpiEnvironment(int argc, char *argv[]) : env_(argc, argv) {
    }

    virtual as::runtime::UnifierGeneratorInterface *GetAction(as::Agent *agent, const as::BeliefAtom &action, const as::Unifier &unifier) override;

    int FindRank(const std::string &name) {
        return name_hash_fn_(name) % world_.size();
    }

public:
    // TODO: Consider making actions that need this friends.
    mpi::environment env_;
    mpi::communicator world_;
    std::vector<mpi::request> requests_;
    std::hash<std::string> name_hash_fn_;
};

class MpiSendBeliefAction : public as::runtime::UnifierGeneratorInterface {
public:
    MpiSendBeliefAction(const as::BeliefAtom &action, as::Agent *agent, MpiEnvironment *env, const as::Unifier &unifier)
        : args_(action.terms), agent_(agent), env_(env), unifier_(unifier), once_(false) {
    }

    bool Next() override {
        if (!once_) {
            once_ = true;

            Message message;
            message.recipient = boost::get<std::string>(args_[0]);
            message.term = args_[1];

            int target_rank = env_->FindRank(message.recipient);

            std::cout << "[[ " << env_->world_.rank() << " --> " << target_rank << " | Belief: ";
            boost::apply_visitor(as::Printer(), args_[1]);
            std::cout << " ]]" << std::endl;

            env_->requests_.push_back(env_->world_.isend(target_rank, 13, message));

            return true;
        }

        return false;
    }

    as::Unifier Current() const override {
        return unifier_;
    }

private:
    std::vector<as::Term> args_;
    as::Agent *agent_;
    MpiEnvironment *env_;
    as::Unifier unifier_;
    bool once_;
};

as::runtime::UnifierGeneratorInterface *MpiEnvironment::GetAction(as::Agent *agent, const as::BeliefAtom &action, const as::Unifier &unifier)  {
    if (action.functor == ".mpi_send_belief") {
        return new MpiSendBeliefAction(action, agent, this, unifier);
    } else {
        return as::runtime::Environment::GetAction(agent, action, unifier);
    }
}

}  // namespace

int main(int argc, char *argv[]) {
    MpiEnvironment env(argc, argv);
    as::runtime::Interpreter interpreter;

    as::grammar::AgentParser<std::string::const_iterator, as::grammar::CommentSkipper<std::string::const_iterator> > parser;
    as::grammar::CommentSkipper<std::string::const_iterator> skipper;

    std::unordered_map<std::string, as::Agent> agents;

    for (int i = 1; i < argc; i++) {
        std::stringstream name;
        name << argv[i];
        name << i;
        if (env.FindRank(name.str()) != env.world_.rank()) {
            continue;
        }

        std::ifstream in(argv[i]);
        if (!in) {
            std::cout << "*** Could not open file! (" << name.str() << ") ***" << std::endl;
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string sourcecode = buffer.str();

        std::string::const_iterator it = sourcecode.begin();
        std::string::const_iterator end = sourcecode.end();

        as::Agent agent;
        agent.name = name.str();

        bool pass = boost::spirit::qi::phrase_parse(it, end, parser, skipper, agent);
        if (!pass || it != end) {
            std::cout << "*** Parser error! (" << agent.name << ") ***" << std::endl;
            return 1;
        }

        agents[agent.name] = agent;
    }

    Message incoming_message;
    Message null;
    mpi::request incoming = env.world_.irecv(mpi::any_source, mpi::any_tag, incoming_message);

    bool more_work = true;

    while (true) {
        more_work = false;
        std::unordered_map<std::string, as::Agent>::iterator it = agents.begin();
        while (it != agents.end()) {
            more_work = interpreter.Run(&env, &(it->second)) || more_work;
            ++it;
        }

        if (!more_work && env.world_.rank() == 0) {
            std::cout << "[[ 0 --> " << (1 % env.world_.size()) << " | Rank 0 done. Please forward if done. ]]" << std::endl;
            env.world_.send(1 % env.world_.size(), 17, null);
        }

        env.requests_.erase(
            mpi::test_some(env.requests_.begin(), env.requests_.end()),
            env.requests_.end());
        bool outgoing_requests = !env.requests_.empty();

        boost::optional<mpi::status> status = incoming.test();

        // TODO: Waiting might be more econimical.
        /* if (!more_work && !status && env.world_.rank() != 0) {
            incoming.wait();
        } */

        if (status) {
            int tag = status->tag();
            if (tag == 13) {
                // Look up the agent by name and add the belief.
                std::unordered_map<std::string, as::Agent>::iterator it = agents.find(incoming_message.recipient);
                if (it == agents.end()) {
                    std::cout << "Recipient: " << incoming_message.recipient << std::endl;
                    for (std::unordered_map<std::string, as::Agent>::const_iterator t = agents.begin(); t != agents.end(); ++t) {
                        std::cout << " - " << t->first << std::endl;
                    }
                    throw std::runtime_error("recipient not found");
                } else {
                    as::BeliefAtom belief = boost::get<as::BeliefAtom>(incoming_message.term);
                    it->second.beliefs.push_back(belief);
                    std::deque<as::IntentionFrame> event;
                    as::IntentionFrame intent(belief);
                    intent.goal_type = as::Plan::kBelief;
                    event.push_back(intent);
                    it->second.intents.push_back(event);
                }
            } else if (tag == 18 || (tag == 17 && !more_work && !outgoing_requests && env.world_.rank() == 0)) {
                std::cout << "[[ " << env.world_.rank() << " --> " << ((env.world_.rank() + 1) % env.world_.size()) << " | Shutting down. ]]" << std::endl;
                if ((env.world_.rank() + 1) % env.world_.size() != 0) {
                    env.world_.send((env.world_.rank() + 1) % env.world_.size(), 18, null);
                }
                return 0;
            } else if (tag == 17 && !more_work && !outgoing_requests) {
                std::cout << "[[ " << env.world_.rank() << " --> " << ((env.world_.rank() + 1) % env.world_.size()) << " | Please forward if also done. ]]" << std::endl;
                env.world_.send((env.world_.rank() + 1) % env.world_.size(), 17, null);
            }
            incoming = env.world_.irecv(mpi::any_source, mpi::any_tag, incoming_message);
        }
    }

    return 0;
}
