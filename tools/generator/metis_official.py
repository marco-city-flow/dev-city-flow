import networkx as nx
import metis
import matplotlib.pyplot as plt
G = nx.Graph()
G.add_node(1)
G.add_node(2)
G.add_edge(1,2)
G.add_edge(3,2)

nx.draw(G, with_labels=True, font_weight='bold')
plt.show()
