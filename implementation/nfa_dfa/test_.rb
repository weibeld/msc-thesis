require 'set'

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