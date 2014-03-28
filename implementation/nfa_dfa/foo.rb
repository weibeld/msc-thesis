require 'set'

class Set
    def print
        s = "{"
        self.each { |e| s << "#{e}," }
        s = s.chomp ","
        s << "}"
    end
end
