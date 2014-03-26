require 'yaml'
require 'set'

class NFA
    def initialize(file)
        yaml = YAML::load_file(file)
        @states = yaml['states']
        @alphabet = yaml['alphabet']
        @transitions = yaml['transitions']
        @start_state = yaml['start_state']
        @accept_states = yaml['accept_states']
    end

    def accept?(string)
        current_states = Set[@start_state]
        string.each_char do |symbol|
            new_states = []
            current_states.each do |state|
                successors = @transitions[state][symbol]
                new_states << successors  unless successors == nil
            end
            current_states = new_states.flatten.to_set
        end
        not (@accept_states & current_states).empty?
    end

    def determinize
        accessible_states = [SubsetState.new([@start_state])]
        transitions_new = {}

        accessible_states.each do |state|

            @alphabet.each do |symbol|
            subset_state = SubsetState.new
                state.each do |component_state|
                    subset_state.add(@transitions[component_state][symbol])
                end
            end
                sucessors.flatten!
            value[symbol] = successors
            accessible_states << successors unless accessible_states.include? successors
            end
            transitions_new[state] = value
        end
    end
end

class SubsetState
    include Enumerable
    extend Forwardable
    def_delegators :@compoenents, :each

    def initialize(states=nil)
        @components = SortedSet.new
        self.add(states)
    end

    def add(states)
        if states.respond_to? "each"
            @components.merge states
        end
    end
end