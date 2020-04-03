#include "command_search.h"
#include <memory>

struct CommandStream {
    ifstream input;
    ofstream output;

    CommandStream(const string& input_name, const string& output_name)
    : input(input_name), output(output_name)
    {
        if(!input) {
            D_FATAL_ERROR("Unable to open input -command-list stream '" + input_name + "'");
        }

        if(!output) {
            D_FATAL_ERROR("Unable to open output -command-list stream '" + output_name + "'");
        }
    }
};

struct Command {
    string type;
    std::vector<std::pair<string, int> > lits;
};

static Command readCommand(istream& o) {
    string type;
    int litlength;
    std::vector<std::pair<string, int> > lits;
    if(!(o >> type)) {
        D_FATAL_ERROR("Cannot read command type");
    }


    if(!(o >> litlength)) {
        D_FATAL_ERROR("Cannot read command number of literals");
    }

    for(int i = 0; i < litlength; ++i) {
        string var;
        int val;
        o >> var >> val;
        lits.push_back(make_pair(var, val));
    }

    if(!o) {
        D_FATAL_ERROR("Failure reading literals");
    }

    return Command{type, lits};
}

static void assignLiterals(const CSPInstance& instance, const std::vector<std::pair<string, int>>& lits)
{
    for(int i = 0; i < lits.size(); ++i) {
        AnyVarRef avr = getAnyVarRefFromString(instance, lits[i].first);
        avr.assign(lits[i].second);
    }
}

static void printAssignment(const CSPInstance& instance, std::unique_ptr<CommandStream>& streams) {
  auto vars = instance.vars.getAllVars();
  streams->output << vars.size() << " ";
  for(auto v : vars) {
    streams->output << instance.vars.getName(v) << " " << getAnyVarRefFromVar(v).assignedValue()
                    << " ";
  }
}

void doCommandSearch(CSPInstance& instance, SearchMethod args) {
  cout << "Switching to command mode" << endl;

  std::unique_ptr<CommandStream> streams(
      new CommandStream(getOptions().commandlistIn, getOptions().commandlistOut));
  while(true) {
    int depth = Controller::getWorldDepth();
    Controller::worldPush();

    Command c = readCommand(streams->input);
    getState().resetSearchCounters();
    assignLiterals(instance, c.lits);
    getQueue().propagateQueue();
    if(getState().isFailed()) {
      streams->output << c.type << " F 0" << std::endl;
    } else {
      if(c.type == "C") {
        streams->output << c.type << " T 0" << std::endl;
      } else if(c.type == "Q") {
        std::cout << "Command mode: Goodbye" << endl;
        exit(0);
      } else if(c.type == "S") {
        SolveCSP(instance, args);
        if(getState().getSolutionCount() > 0) {
          streams->output << "T ";
          printAssignment(instance, streams);
          streams->output << std::endl;
        } else {
          streams->output << "F 0" << std::endl;
        }
      }
    }

    Controller::worldPopToDepth(depth);
  }
}