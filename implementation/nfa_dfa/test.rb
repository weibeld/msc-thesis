a = [1,2]
a.each do |elt|
    a << elt + 1 unless elt >= 10
end

print a
