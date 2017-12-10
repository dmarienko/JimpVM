package java.util;

public class Vector {
    Object[] objects = new Object[0];

    public Vector() {
        objects = new Object[0];
//        System.out.println("Vector constructed.");
    }

    public int size() {
        return objects.length;
    }

    public boolean isEmpty() {
        if (objects.length < 1) return true;
        return false;
    }

    public boolean contains(Object ob) {
        for (int i = 0; i < objects.length; i++) {
            Object o = objects[i];
            if (o.equals(ob)) return true;
        }
        return false;
    }

    public Object elementAt(int i) {
        try {
            return objects[i];
        }
        catch (Exception e) {
        }
        return null;
    }

    public void addElement(Object o) {
        int size = objects.length;
        Object[] ot = new Object[size + 1];
        for (int i = 0; i < size + 1; i++) {
            if (i < size) {
                ot[i] = objects[i];
            }
            else {
                ot[i] = o;
            }
        }
        objects = ot;
    }

    public void removeElementAt(int ind) {
        if (ind != -1) {
            try {
                Object[] ot = new Object[objects.length - 1];
                for (int i = 0; i < ind; i++) {
                    ot[i] = objects[i];
                }
                for (int i = ind + 1; i < objects.length; i++) {
                    ot[i - 1] = objects[i];
                }
                objects = ot;
            }
            catch (Exception e) { }
        }
    }

    public void removeElement(Object o) {
        int ind = -1;
        for (int i = 0; i < objects.length; i++) {
            if (objects[i].equals(o)) ind = i;
        }
        removeElementAt(ind);
    }

    public void removeAllElements(){
      while(size()>0){
        removeElementAt(size()-1);
      }
    }

}
