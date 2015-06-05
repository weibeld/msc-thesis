image(x=1:10, y=1:11, z=m, col=terrain.colors(3), xaxt="n", yaxt="n")
par(xpd=TRUE)
text(y=12, x=1:10, labels=Float(seq(0.1, 1, 0.1)))
text(y=12, x=1:10, labels=Float(seq(0.1, 1, 0.1)))


# axis(1, at=1:10, labels=Float(seq(0.1, 1, 0.1)))
axis(2, at=1:11, labels=Float(seq(3, 1, -0.2)), las=1) 
par(xaxp=c(0.5, 10.5, 10), yaxp=c(0.5, 11.5, 11))
grid(col="black", lty="solid") 
