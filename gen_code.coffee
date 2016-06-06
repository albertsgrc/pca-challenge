matrices = [
    [
        [1, -2, 2],
        [2, -1, 2],
        [2, -2, 3]
    ],
    [
        [1, 2, 2],
        [2, 1, 2],
        [2, 2, 3]
    ],
    [
        [-1, 2, 2],
        [-2, 1, 2],
        [-2, 2, 3]
    ]
]

newMatrices = []

times = (m1, m2) ->
    res = [
        [0,0,0],
        [0,0,0],
        [0,0,0]
    ]
    for i in [0...3]
        for j in [0...3]
            for k in [0...3]
                 res[i][j] += m1[i][k]*m2[k][j]

    res

for matrix1 in matrices
    for matrix2 in matrices
        newMatrices.push times(matrix1, matrix2)


s = "unsigned int\n"
for i in [0...matrices.length+ newMatrices.length]
    s += "x#{i}, y#{i}, z#{i},\n"
    if i is matrices.length+ newMatrices.length - 1
        s = s[...-2]

s += ";\n\n"

for m,i in matrices
    s += "z#{i} = #{m[2][0]}*x + #{m[2][1]}*y + #{m[2][2]}*z;\n"
    s += "if (z#{i} <= N) { ++mem[z#{i}]; }\n"


for m,i in newMatrices
    i += matrices.length
    s += "z#{i} = #{m[2][0]}*x + #{m[2][1]}*y + #{m[2][2]}*z;\n"
    s +=
        """
            if (z#{i} <= N) {
                x#{i} = #{m[0][0]}*x + #{m[0][1]}*y + #{m[0][2]}*z;
                y#{i} = #{m[1][0]}*x + #{m[1][1]}*y + #{m[1][2]}*z;
                ++mem[z#{i}]; compute_triples_from(x#{i},y#{i},z#{i});
            }\n
        """


console.log s
