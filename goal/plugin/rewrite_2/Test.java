import java.util.List;
import java.util.LinkedList;
import java.util.HashSet;
import java.util.Collection;

public class Test {
  public static void main(String[] args) {
    Component c1 = new Component(new StateSet(singleEltList(1)), 1);
    Component c2 = new Component(new StateSet(singleEltList(2)), 2);
    Component c3 = new Component(new StateSet(singleEltList(3)), 2);
    Component c4 = new Component(new StateSet(singleEltList(4)), 0);
    Component c5 = new Component(new StateSet(singleEltList(5)), 2);
    Component c6 = new Component(new StateSet(singleEltList(6)), 2);
    Component c7 = new Component(new StateSet(singleEltList(7)), 1);
    Component c8 = new Component(new StateSet(singleEltList(8)), 2);
    Component c9 = new Component(new StateSet(singleEltList(9)), 2);
    Component c10 = new Component(new StateSet(singleEltList(10)), 0);
    Component c11 = new Component(new StateSet(singleEltList(11)), 1);
    
    List<Component> list = new LinkedList<Component>();
    list.add(c1);
    list.add(c2);
    // list.add(c3);
    // list.add(c4);
    // list.add(c5);
    // list.add(c6);
    // list.add(c7);
    // list.add(c8);
    // list.add(c9);
    // list.add(c10);
    // list.add(c11);
    
    // System.out.println("The initial list is :"+list);
    // List mergedList = mergeComponents(list);
    // System.out.println("The Reverse List is :"+mergedList);

    System.out.println("The initial list is :"+list);
    //invertList(list);
    List mergedList = mergeComponents(list);
    System.out.println("The Reverse List is :"+mergedList);
  }

  // For the merging optimisation (-m)
  public static LinkedList<Component> mergeComponents(List<Component> list) {
    return invertList(merged(invertList(list)));
  }

  private static LinkedList<Component> merged(List<Component> list) {
    if (list.size() <= 1) return new LinkedList<Component>(list);
    return push(head(list), merged(tail(list)));
  }

  private static LinkedList<Component> push(Component c, LinkedList<Component> list) {
    Component h = list.get(0);
    if      (c.color() == 1 && h.color() == 1) list.set(0, merge(c, h, 1));
    else if (c.color() == 2 && h.color() == 2) list.set(0, merge(c, h, 2));
    else if (c.color() == 1 && h.color() == 2) list.set(0, merge(c, h, 2));
    else list.add(0, c);
    return list;
  }

  private static Component merge(Component c1, Component c2, int color) {
    StateSet states = new StateSet();
    states.addAll(c1.stateSet());
    states.addAll(c2.stateSet());
    Component merged = new Component(states, color);
    // if (c1.isStar() || c2.isStar()) merged.setStar();
    // if (c1.isSuppressed() || c2.isSuppressed()) merged.setSuppressed();
    return merged;
  }

  public static class Component {
    private StateSet stateSet;
    private int color;
    public Component(StateSet stateSet, int color) {
      this.stateSet = stateSet;
      this.color = color;
    }
    public StateSet stateSet() {
      return stateSet;
    }
    public int color() {
      return color;
    }
    public String toString() {
      if (color == 0) return "{" + printStateSet() + "}";
      else if (color == 1) return "(" + printStateSet() + ")";
      else if (color == 2) return "[" + printStateSet() + "]";
      else return "";
    }
    private String printStateSet() {
      String s = "";
      for (int state : stateSet) {
        s += state + ",";
      }
      s = s.substring(0, s.length()-1);
      return s;
    }
  }

  public static class StateSet extends HashSet<Integer> {
    public StateSet() {
      super();
    }
    public StateSet(Collection<? extends Integer> c) {
      super(c);
    }
    
  }

  public static class Foo {
    String name;
    int age;

    public Foo(String name, int age) {
      this.name = name;
      this.age = age;
    }
  }

  public static LinkedList singleEltList(Object elt) {
    LinkedList list = new LinkedList();
    list.add(elt);
    return list;
  }

  /* Prolog-like list inversion */
  public static <T> LinkedList<T> invertList(List<T> list) {
    if (list.isEmpty()) return new LinkedList<T>();
    LinkedList<T> invertedList = invertList(tail(list));
    invertedList.add(head(list));
    return invertedList;
  }

  // public static void invertList(List<Component> list) {
  //   if (list.isEmpty()) list = new LinkedList();
  //   else {
  //   Component head = (Component) head(list);
  //   invertList(tail(list));
  //   list.add(head);
  //   }
  // }

  /* Prolog-like head and tail of a list. */
  // Return head of list. If list is empty, then Error.
  public static <T> T head(List<T> list) {
    return list.get(0);
  }
  // Returns tail of list. If list size is 1, returns empty list. If list is
  // empty, then Error.
  public static <T> List<T> tail(List<T> list) {
    return list.subList(1,list.size()); // from: inclusive, to: exclusive
  }
}