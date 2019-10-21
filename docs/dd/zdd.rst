ZDD
===

A zero-suppressed decision diagram (ZDD) is a graph-based representation of a
finite family of finite subsets. Given a set of variables
:math:`X = {x_1, \dots, x_n}`, a ZDD is a directed acyclic graph with
non-terminal verices :math:`N`, also called decision vertices, and two terminal
vertices :math:`\top` and :math:`\bot`. Each vertex :math:`v \in N` is
associated with variable :math:`V(v) \in X` and has two successor vertices
:math:`{\rm HI}(v), {\rm LO}(v) \in N \cup \{\top, \bot\}`. The vertices on a
directed path to a terminal vertex follow a fixed variable order which
guarantees the canonicity of the ZDD.

Given two ZDDs :math:`f` and :math:`g`, the following list of operations is
part of what is called a ZDD family algebra. These operations are implemented
in the package.


.. |diff| replace:: :math:`f \;\backslash\; g = \{\alpha \, | \, \alpha \in f \; \text{and} \; \alpha \notin g\}`
.. |inter| replace:: :math:`f \cap g = \{\alpha \, | \, \alpha \in f \; \text{and} \; \alpha \in g\}`
.. |join| replace:: :math:`f \sqcup g = \{\alpha \cup \beta \, | \, \alpha \in f \; \text{and} \; \beta \in g\}`
.. |nonsup| replace:: :math:`f \searrow g = \{\alpha \in f\, | \, \beta \in g \; \text{implies} \; \alpha \nsupseteq \beta\}`
.. |union| replace:: :math:`f \cup g = \{\alpha \, | \, \alpha \in f \; \text{or} \; \alpha \in g\}`

+--------------------------------+----------+
| Choose                         |          |
+--------------------------------+----------+
| Difference                     | |diff|   |
+--------------------------------+----------+
| Intersection                   | |inter|  |
+--------------------------------+----------+
| Join                           | |join|   |
+--------------------------------+----------+
| Nonsupersets                   | |nonsup| |
+--------------------------------+----------+
| Tautology                      |          |
+--------------------------------+----------+
| Union                          | |union|  |
+--------------------------------+----------+

ZDD base
--------

.. doxygenclass:: bill::zdd_base
   :members: zdd_base, bottom, top, elementary, ref, deref, garbage_collect
   :no-link:
