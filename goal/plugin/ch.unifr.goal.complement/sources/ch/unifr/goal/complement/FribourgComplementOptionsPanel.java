package ch.unifr.goal.complement;

import javax.swing.Box;
import javax.swing.JCheckBox;
import java.awt.GridLayout;
import javax.swing.border.EmptyBorder;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import org.svvrl.goal.gui.pref.OptionsPanel;
import org.svvrl.goal.core.Properties;


/*----------------------------------------------------------------------------*
 * The JPanel appearing in the options dialog when complementing an automaton,
 * and in the GOAL preferences under Preferences > Complementation >
 * FribourgConstruction.
 * Contains a checkbox for every option of the Fribourg Construction. Returns a
 * FribourgOptions with options set according to the state of the checkboxes.
 * On object creation, the checkboxes are set according to the preference values
 * in the preference file.
 * Daniel Weibel, 31.07.2014
 *----------------------------------------------------------------------------*/
// JPanel > OptionsPanel
public class FribourgComplementOptionsPanel extends OptionsPanel<FribourgOptions> {

  // A checkbox for every option
  private final JCheckBox complCheckBox;
  private final JCheckBox right2IfComplCheckBox;
  private final JCheckBox mergeCheckBox;
  private final JCheckBox reduce2CheckBox;
  private final JCheckBox brackCheckBox;
  private final JCheckBox pruneCheckBox;

  /* Constructor */
  public FribourgComplementOptionsPanel() {

    // Layout of panel in OptionsDialog
    this.setLayout(new GridLayout(0,1));
    this.setBorder(new EmptyBorder(10,10,5,10)); // (top,left,bottom,right)

    // vBox: vertical stack of hBoxes
    Box vBox = createYBox();
    Box hBox = createXBox();

    // Option: make input automaton complete (-c)
    complCheckBox = new JCheckBox("Make input automaton complete if it is incomplete (-c)");
    complCheckBox.setSelected(FribourgOptions.getComplPref());
    vBox.add(hBox.add(complCheckBox));

    // Option: rightmost colour 2 pruning if input automaton is complete (-r2ifc)
    right2IfComplCheckBox = new JCheckBox("If input automaton is complete, apply rightmost colour 2 optimisation (-r2ifc)");
    right2IfComplCheckBox.setSelected(FribourgOptions.getRight2IfComplPref());
    vBox.add(hBox.add(right2IfComplCheckBox));

    // Option: component merging optimisation (-m)
    mergeCheckBox = new JCheckBox("Apply component merging optimisation (-m)");
    mergeCheckBox.setSelected(FribourgOptions.getMergePref());
    vBox.add(hBox.add(mergeCheckBox));

    // Option: colour 2 reduction optimisation (-m2). Can only be selected if
    // the -m option is also selected.
    reduce2CheckBox = new JCheckBox("Apply colour 2 component reduction optimisation (-m2)");
    // If merge optimisation is on, initialise checkbox normally
    if (mergeCheckBox.isSelected()) reduce2CheckBox.setSelected(FribourgOptions.getReduce2Pref());
    // Else, set checkbox unchecked (default) and non-editable
    else reduce2CheckBox.setEnabled(false);
    vBox.add(hBox.add(reduce2CheckBox));
    // Whenever merge is ticked, make colour 2 editable, and vice versa
    mergeCheckBox.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        if (mergeCheckBox.isSelected())
          reduce2CheckBox.setEnabled(true);
        else {
          reduce2CheckBox.setSelected(false);
          reduce2CheckBox.setEnabled(false);
        }
      }
    });

    // Option: use the bracket notation
    brackCheckBox = new JCheckBox("Use the \"bracket notation\" for labelling states (-b)");
    brackCheckBox.setSelected(FribourgOptions.getBrackPref());
    vBox.add(hBox.add(brackCheckBox));

    // Option: prune unreachable and dead states from the output automaton
    pruneCheckBox = new JCheckBox("Remove unreachable and dead states from the output automaton (-r)");
    pruneCheckBox.setSelected(FribourgOptions.getPrunePref());
    vBox.add(hBox.add(pruneCheckBox));

    // Add the vertical stack of hBoxes
    this.add(vBox);
  }

  /* Create a FribourgOptions according to the state of the checkboxes */
  @Override // Abstract method of OptionsPanel
  public FribourgOptions getOptions() {
    FribourgOptions options = new FribourgOptions();
    options.setCompl(complCheckBox.isSelected());
    options.setRight2IfCompl(right2IfComplCheckBox.isSelected());
    options.setMerge(mergeCheckBox.isSelected());
    options.setReduce2(reduce2CheckBox.isSelected());
    options.setBrack(brackCheckBox.isSelected());
    options.setPrune(pruneCheckBox.isSelected());
    return options;
  }

  /* Set the checkboxes according to the option default values, which are in
   * the preference file. Used when the user clicks on 'Reset' in 'Preferences..
   * > Complementation > Fribourg Construction' (currently not implemented */
  @Override // Abstract method of OptionsPanel
  public void reset() {
    complCheckBox.setSelected(FribourgOptions.getComplDefault());
    right2IfComplCheckBox.setSelected(FribourgOptions.getRight2IfComplDefault());
    mergeCheckBox.setSelected(FribourgOptions.getMergeDefault());
    reduce2CheckBox.setSelected(FribourgOptions.getReduce2Default());
    brackCheckBox.setSelected(FribourgOptions.getBrackDefault());
    pruneCheckBox.setSelected(FribourgOptions.getPruneDefault());
  }
}
