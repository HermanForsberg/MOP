# en lösning på a)

def random_graph(n, p):
    # Skapa en nxn-matris med endast nollor.
    adjacency_matrix = np.zeros((n, n))
    
    # För varje par av noder (i, j) i grafen, med i < j:
    for i in range(n):
        for j in range(i + 1, n):
            
            prob = np.random.random()
            adjacency_matrix[i][j] = prob
            adjacency_matrix[j][i] = prob
            
            # Om sannolikheten är mindre än p, lägg till en kant i grafen.
        
    adjacency_matrix = adjacency_matrix > p
    adjacency_matrix = adjacency_matrix * 1
    # Returnera grannmatrisen för den slumpade grafen.
    return adjacency_matrix


ary = random_graph(10, 0.5)
print(ary)
