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
        optimal_vector_matrix = np.matrix(np.append(self.optimal_vector, [1], 1))

        # The error Quadric of this target v becomes the cost of contracting that pair
        # print("Error Quadric Matrices")
        # print("v'", optimal_vector_matrix)
        # print("Q1", v1.Q)
        # print("Q2", v2.Q)
        # print("-------------------")
        self.cost = (optimal_vector_matrix * (v1.Q + v2.Q) * optimal_vector_matrix.T).item()
        
    def __repr__(self):
        return "({}, {}) ~ Cost: {cost}".format(*self.vertex_pair, cost=self.cost)

class VertexPairPriorityQueue(object):

    def __init__(self, vertices):

        self.queue = []
        self.entry_finder = {}
        self.counter = itertools.count()
        for vertex in vertices:
            for edge in vertex.edges.values():
                if (edge.vertex_pair not in self.entry_finder):
                    self.add(edge)

    def add(self, edge):
        if (edge.vertex_pair in self.entry_finder):
            self.remove(edge)
        
        count = next(self.counter)
        entry = [edge.cost, count, edge]
        self.entry_finder[edge.vertex_pair] = entry
        heapq.heappush(self.queue, entry)

    def remove(self, edge):
        try:
            entry = self.entry_finder.pop(edge.vertex_pair)
        except KeyError:
            pass
        else:
            entry[-1] = None

    def pop(self):
        while self.queue:
            cost, count, edge = heapq.heappop(self.queue)
            
            if edge and edge.vertex_pair in self.entry_finder:
                del self.entry_finder[edge.vertex_pair]
                return edge
        raise KeyError('Queue is Empty')

    def empty(self):
        return False if self.queue else True


class Mesh(object):
    def __init__(self):
        self.face_list = []
        self.vertex_list = []


        
    def load(self, filename):
        with open(filename) as f:
            for line in f:
                command, *args = line.split()
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
                edge.computeOptimalVector()

    def _contract(self, edge):
        """Return v1 and remove v2
        relocate v1 to its optimal position
        """

        # v1 to be kept, v2 to be removed

        v1, v2 = edge.vertex_pair
        if v1 not in self.vertex_list or v2 not in self.vertex_list:
            # raise KeyError("Vertex is not in the list")
            return

        # print("v1 E: ", len(v1.edges))
        # print("v2 E: ", len(v2.edges))
        # print("v1 F: ", len(v1.faces))
        # print("v2 F: ", len(v2.faces))

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



        # every occurance of v2 in valid pairs is replaced by v1, (remove duplicate)
        affected_vertices = [v for face in v1.faces for v in face.vertices if v != v1 and v != v2]
        
        for v in affected_vertices:
            v.edges[edge.vertex_pair] = edge

        for edge in v1.edges.values():
            edge.computeOptimalVector()
            self.pq.add(edge)
            
        for edge in v2.edges.values():
            v1.edges[edge.vertex_pair] = edge
            self.pq.add(edge)

        return v1

    def simplify(self, face_count):
        # Step 1: Compute the Q matrices for all the initial vertices.
        self._initQmatrices()

        # Step 2: Select all valid pairs
        self._selectValidVertexPair()

        # Step 3: Compute the optimal contracti1on target v for each valid pair
        self._computeContractionTarget()

        # Step 4: Place all the pairs in a min-heap keyed on cost
        self.pq = VertexPairPriorityQueue(self.vertex_list)


        # Step 5: Iteratively remove the pair (v1, v2) of least cost from the heap,
        # contract this pair, and update the costs of all valid pairs involving v1 .
        while (not self.pq.empty() and len(self.face_list) > face_count):
            edge = self.pq.pop()
            v = self._contract(edge)       
            print("FL", len(self.face_list))
            print("VL", len(self.vertex_list))
            print("PQQ", len(self.pq.queue))
            print("PQE", len(self.pq.entry_finder))
            print()

def ball():    
    mesh = Mesh()
    mesh.load('data/ball.obj')
    mesh.simplify(78)
    mesh.save('data/ball2.obj')

def ball2():    
    mesh = Mesh()
    mesh.load('data/ball.obj')
    mesh.simplify(10)
    mesh.save('data/ball3.obj')

def cow():
    mesh = Mesh()
    mesh.load('data/cow5804.mesh')
    mesh.simplify(500)
    mesh.save('data/cow500.mesh')

def sphere():
    mesh = Mesh()
    mesh.load('data/sphere.obj')
    mesh.simplify(18)
    mesh.save('data/sphere19.obj')

def tetrahedron():
    mesh = Mesh()
    mesh.load('data/try.obj')
    mesh.simplify(5)
    mesh.save('data/try123.obj')

def plane():
    mesh = Mesh()
    mesh.load('data/plane.obj')
    mesh.simplify(7)
    mesh.save('data/plane123.obj')

def main():
    #ball()
    #ball2()
    #sphere()
    cow()
    #tetrahedron()
    #plane()

if __name__ == '__main__':
    main()
