import sys
import itertools
import numpy as np
import heapq


class Vertex(object):
    def __init__(self, x, y, z):
        self.vector = np.array([x, y, z])
        self.faces = set()
        self.edges = {}

        self.error = np.matrix([])

        self.Q = np.asmatrix(np.zeros((4,4)))

    def __repr__(self):
        return "({}, {}, {})".format(*self.vector)
        

class Face(object):
    def __init__(self, a, b, c):
        self.vertices = (a,b,c)

    def getNormalMatrix(self):
        AB = self.vertices[1].vector - self.vertices[0].vector
        AC = self.vertices[2].vector - self.vertices[0].vector
        normal = np.cross(AB, AC)
        normal = normal / np.linalg.norm(np.cross(AB, AC))
        return np.append(normal, [1], 1)

    def getKmatrix(self):
        n = self.getNormalMatrix()
        return n * n.T

    def __repr__(self):
        return "{}, {}, {}".format(*self.vertices)

class Edge(object):
    def __init__(self, v1, v2):
        self.vertex_pair = frozenset([v1, v2])

    def computeOptimalVector(self):
        v1, v2 = self.vertex_pair
        self.optimal_vector = 0.5 * (v1.vector + v2.vector)
        
    def computeCost(self):
        v1, v2 = self.vertex_pair
        optimal_vector_matrix = np.matrix(np.append(self.optimal_vector, [1], 1))
        self.cost = (optimal_vector_matrix * (v1.Q + v2.Q) * optimal_vector_matrix.T).item()
        
    def __repr__(self):
        return "({}, {}) ~ Cost: {cost}".format(*self.vertex_pair, cost=self.cost)

class VertexPairPriorityQueue(object):

    def __init__(self):

        self.queue = []
        self.entry_finder = {}
        self.counter = itertools.count()
        
    def add(self, edge):
        if (edge.vertex_pair in self.entry_finder):
            self.remove(edge)
        
        count = next(self.counter)
        entry = [edge.cost, count, edge]
        self.entry_finder[edge.vertex_pair] = entry
        heapq.heappush(self.queue, entry)

    def remove(self, edge):
        entry = self.entry_finder.pop(edge.vertex_pair)
        entry[-1] = None

    def pop(self):
        while self.queue:
            cost, count, edge = heapq.heappop(self.queue)
            
            if edge and edge.vertex_pair in self.entry_finder:
                if (edge.cost != cost):
                    self.add(edge)
                else:
                    del self.entry_finder[edge.vertex_pair]
                    return edge
        raise KeyError('Queue is Empty')


class Mesh(object):
    def __init__(self):
        self.face_list = []
        self.vertex_list = []
        self.pq = VertexPairPriorityQueue()
        
    def load(self, filename):
        with open(filename) as f:
            for line in f:
                if line.split():
                    command, *args = line.split()
                else:
                    command = ''

                if command == 'v':
                    args = [float(arg) for arg in args]
                    self.vertex_list.append(Vertex(args[0], args[1], args[2]))
                elif command == 'f':
                    args = [int(arg) - 1 for arg in args]
                    a = self.vertex_list[args[0]]
                    b = self.vertex_list[args[1]]
                    c = self.vertex_list[args[2]]
                    face = Face(a, b, c)
                    self.face_list.append(face)
                    a.faces.add(face)
                    b.faces.add(face)
                    c.faces.add(face)

    def save(self, filename):
        with open(filename, 'w') as f:
            for vertex in self.vertex_list:
                print('v {} {} {}'.format(*vertex.vector), file=f)

            for face in self.face_list:
            
                vertex_indexes = [self.vertex_list.index(vertex) + 1 for vertex in face.vertices]
            
                print('f {} {} {}'.format(*vertex_indexes), file=f)

            print('# {} faces'.format(len(self.face_list)), file=f)


    def _computeQmatrix(self, vertex):
        vertex.Q = np.asmatrix(np.zeros((4,4)))
        for face in vertex.faces:
            vertex.Q = vertex.Q + face.getKmatrix()

    def _initQmatrices(self):
        for vertex in self.vertex_list:
            self._computeQmatrix(vertex)


    def _selectValidVertexPair(self):
        for face in self.face_list:
            edges = [
                Edge(face.vertices[0], face.vertices[1]),
                Edge(face.vertices[1], face.vertices[2]),
                Edge(face.vertices[2], face.vertices[0]),
            ]

            face.vertices[0].edges[edges[0].vertex_pair] = edges[0]
            face.vertices[1].edges[edges[0].vertex_pair] = edges[0]
            face.vertices[1].edges[edges[1].vertex_pair] = edges[1]
            face.vertices[2].edges[edges[1].vertex_pair] = edges[1]
            face.vertices[2].edges[edges[2].vertex_pair] = edges[2]
            face.vertices[0].edges[edges[2].vertex_pair] = edges[2]

        
    def _computeContractionTarget(self):
        for vertex in self.vertex_list:
            for edge in vertex.edges.values():
                self._recomputeEdge(edge)

    def _recomputeEdge(self, edge):
        edge.computeOptimalVector()
        edge.computeCost()
        self.pq.add(edge)


    def _collapseEdge(self, edge):

        # v1 to be kept, v2 to be removed

        v1, v2 = edge.vertex_pair
        if v1 not in self.vertex_list or v2 not in self.vertex_list:
            return

        # Remove v2 from vertex_list
        self.vertex_list.remove(v2)

        # v1 relocate to its optimal position.
        v1.vector = edge.optimal_vector


        # v1 acquire all the edges linked to v2
        for face in v2.faces:
            # replace v2 to v1 for each vertex of the faces
            vertices = [v1 if vertex == v2 else vertex for vertex in face.vertices]
            
            # if there exists a face containing the same vertices as face.vertices
            # then remove such faces from the face_list
            if len(set(vertices)) != len(vertices):
                # Duplicate Face -> Remove
                try:
                    self.face_list.remove(face)
                except ValueError:
                    # face has been removed 
                    pass
            else:
                face.vertices = vertices
                v1.faces.add(face)


        # Every Occurance of v2 in all valid pairs is replaced by v1
        for vertex_pair, v2_edge in v2.edges.items():
            if v2_edge != edge:
                vp = frozenset({v1 if v == v2 else v for v in vertex_pair}) 
                if len(vp) == 2: 
                    v2_edge.vertex_pair = vp
                    v1.edges[vertex_pair] = v2_edge

                    # BUG: Recomputing Edge Cost causes a deformation in the meshes
                    # Need to Inspect more details on the computation of Cost
                    # self._recomputeEdge(v2_edge)
        return v1

    def simplify(self, face_count):

        print("Original Face count: ", len(self.face_list))

        # Step 1: Compute the Q matrices for all the initial vertices.
        print("initializing Matrices")
        self._initQmatrices()

        # Step 2: Select all valid pairs
        print("Selecting All Edges")
        self._selectValidVertexPair()

        # Step 3: Compute the optimal contracti1on target v for each valid pair
        print("Computing Optimal Collapse Point")
        self._computeContractionTarget()

        # Step 4: Place all the pairs in a min-heap keyed on cost
        print("Enqueing Vertices to Heap")
        for vertex in self.vertex_list:
            for edge in vertex.edges.values():
                if (edge.vertex_pair not in self.pq.entry_finder):
                    self.pq.add(edge)

        # Step 5: Iteratively remove the pair (v1, v2) of least cost from the heap,
        # contract this pair, and update the costs of all valid pairs involving v1 .
        print("Collapsing Edges")
        while (len(self.face_list) > face_count):
            try:
                edge = self.pq.pop()
            except KeyError:
                print("Unable to reduce Further")
                break
            else:
                v = self._collapseEdge(edge)
        else:
            print("Mesh Simplified Successful")
        print("Face Reduced to: ", len(self.face_list))


def main():
    mesh = Mesh()
    mesh.load(sys.argv[1])
    mesh.simplify(int(sys.argv[3]))
    mesh.save(sys.argv[2])


if __name__ == '__main__':

    print("""

            ======================
              Mesh Simplification 
            ======================

        """)


    if len(sys.argv) != 4:
        print("Parameters: <input-file> <output-file> <face-count>")
    else:
        main()
