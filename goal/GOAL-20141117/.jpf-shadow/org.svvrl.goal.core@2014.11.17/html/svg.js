var target = null;
var sx = 0;
var sy = 0;

function isCircleNode(node) {
  return node != null && node.nodeName == "circle";
}

function isRectNode(node) {
  return node != null && node.nodeName == "rect";
}

function isStateNode(node) {
  return (isCircleNode(node) || isRectNode(node)) && node.getAttribute("type") == "state";
}

function isTextNode(node) {
  return node != null && node.nodeName == "text";
}

function isPathNode(node) {
  return node != null && node.nodeName == "path";
}
    
function isPolygonNode(node) {
  return node != null && node.nodeName == "polygon";
}

function getX(node) {
  if (isCircleNode(node))
    return parseFloat(node.getAttribute("cx"));
  else if (isRectNode(node) || isTextNode(node))
    return parseFloat(node.getAttribute("x"));
}

function setX(node, x) {
  if (isCircleNode(node))
    return node.setAttribute("cx", x);
  else if (isRectNode(node) || isTextNode(node))
    return node.setAttribute("x", x);
}

function getY(node) {
  if (isCircleNode(node))
    return parseFloat(node.getAttribute("cy"));
  else if (isRectNode(node) || isTextNode(node))
    return parseFloat(node.getAttribute("y"));
}

function setY(node, y) {
  if (isCircleNode(node))
    return node.setAttribute("cy", y);
  else if (isRectNode(node) || isTextNode(node))
    return node.setAttribute("y", y);
}

function getWidth(node) {
  if (isCircleNode(node))
    return parseFloat(node.getAttribute("r"));
  else if (isRectNode(node))
    return parseFloat(node.getAttribute("width"));
}

function getHeight(node) {
  if (isCircleNode(node))
    return parseFloat(node.getAttribute("r"));
  else if (isRectNode(node))
    return parseFloat(node.getAttribute("height"));
}
 
function mouseDown(evt) {
  target = evt.target;
  if (target == null)
    return;
  if (target.nodeName == "g") {
    var parent = target;
    for (var i = 0; !isStateNode(target) && i < parent.childNodes.length; i++) {
      target = parent.childNodes[i];
    }
  } else {
    var parent = target.parentNode;
    for (var i = 0; !isStateNode(target) && i < parent.childNodes.length; i++) {
      target = parent.childNodes[i];
    }
  }
      
  /* Determine the center. */
  sx = getX(target);
  sy = getY(target);
  if (isRectNode(target)) {
    sx += getWidth(target) / 2;
    sy += getHeight(target) / 2;
  }
}
    
function mouseUp(evt) {
  target = null;
}
    
function mouseMove(evt) {
  if (target == null)
    return;
  var dx = evt.clientX - sx;
  var dy = evt.clientY - sy;
  var state = target.parentNode;
  moveState(state, dx, dy);
  moveTransitions(state, dx, dy);
}

function getTransitionsFromState(sid) {
  var ret = new Array();
  var trans = document.getElementById("transitions").childNodes;
  for (var i in trans) {
    if (trans[i].nodeName != "g")
      continue;
    if (trans[i].getAttribute("from") == sid)
      ret.push(trans[i]);
  }
  return ret;
}

function getTransitionsToState(sid) {
  var ret = new Array();
  var trans = document.getElementById("transitions").childNodes;
  for (var i in trans) {
    if (trans[i].nodeName != "g")
      continue;
    if (trans[i].getAttribute("to") == sid)
      ret.push(trans[i]);
  }
  return ret;
}
    
function movePoint(node, xn, yn, dx, dy) {
  var x = parseFloat(node.getAttribute(xn)) + dx;
  var y = parseFloat(node.getAttribute(yn)) + dy;
  node.setAttribute(xn, x);
  node.setAttribute(yn, y);      
  return [x, y];
}
    
function movePoints(node, an, dx, dy) {
  var points = node.getAttribute(an).split(" ");
  for (var i = 0; i < points.length; i++) {
    var cs = points[i].split(",");
    var x = parseFloat(cs[0]);
    var y = parseFloat(cs[1]);
    points[i] = (x + dx) + "," + (y + dy);
  }
  node.setAttribute(an, points.join(" "));
}

function moveState(state, dx, dy) {
  var childs = state.childNodes;
  for (var i in childs) {
    var child = childs[i];

    var xn = null;
    var yn = null;
    if (isCircleNode(child)) {
      var cs = movePoint(child, "cx", "cy", dx, dy);
      if (isStateNode(child)) {
        sx = cs[0];
        sy = cs[1];
      }
    } else if (isRectNode(child)) {
      var cs = movePoint(child, "x", "y", dx, dy);
      if (isStateNode(child)) {
        sx = cs[0] + getWidth(child) / 2;
        sy = cs[1] + getWidth(child) / 2;
      }
    } else if (isTextNode(child)) {
      movePoint(child, "x", "y", dx, dy);
    } else if (isPolygonNode(child)) {
      movePoints(child, "points", dx, dy);
    }
  }
}

function moveTransitions(state, dx, dy) {
    var id = state.getAttribute("id");

    var trans = getTransitionsFromState(id);
    for (var i in trans)
      moveTransition(trans[i], id, dx, dy);

    trans = getTransitionsToState(id);
    for (var i in trans) {
      /* Skip self-loops. */
      if (trans[i].getAttribute("from") == id)
        continue;
      moveTransition(trans[i], id, dx, dy);
    }
}
    
function moveTransition(tran, sid, dx, dy) {
  var from = tran.getAttribute("from");
  var to = tran.getAttribute("to");

  var childs = tran.childNodes;
  for (var i in childs) {
    var child = childs[i];
    if (isPathNode(child)) {
      var points = child.getAttribute("d").split(" ");
      for (var j in points) {
        var point = points[j];
        var mod = point.charAt(0) == "M" || point.charAt(0) == "Q" ? point.charAt(0) : "";
        var cods = (mod.length == 0 ? point : point.substr(1)).split(",");
        var x = parseFloat(cods[0]);
        var y = parseFloat(cods[1]);

        if (from == sid && j == 0 || from == sid && to == sid && j == 1 || to == sid && j == 2)
          points[j] = mod + (x + dx) + "," + (y + dy);
        else if (j == 1)
          points[j] = mod + (x + dx / 2) + "," + (y + dy / 2);
      }
      child.setAttribute("d", points.join(" "));
    } else if (isTextNode(child)) {
      if (from != sid || to != sid) {
        dx = dx / 2;
        dy = dy / 2;
      }
      movePoint(child, "x", "y", dx, dy);

      var transforms = child.getAttribute("transform").split(" ");
      for (var j in transforms) {
        var transform = transforms[j];
        if (!transform.match(/rotate\(.*\)/))
          continue;
        var ts = transform.substring(7, transform.length - 1).split(",");
        ts[1] = parseFloat(ts[1]) + dx;
        ts[2] = parseFloat(ts[2]) + dy;
        transforms[j] = "rotate(" + ts.join(",") + ")";
      }
      child.setAttribute("transform", transforms.join(" "));
    } 
  }
}