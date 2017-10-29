## Relationship between SVD and PCA. How to use SVD to perform PCA?

Let the data matrix $\mathbf X$ be of $n×p$ size, where n is the number of samples and p is the number of variables. Let us assume that it is *centered*, i.e. column means have been subtracted and are now equal to zero.

If we now perform singular value decomposition of $X$, we obtain a decomposition

$\mathbf X = \mathbf U \mathbf S \mathbf V^\top$

Principal components are given by $\mathbf X \mathbf V = \mathbf U \mathbf S \mathbf V^\top \mathbf V = \mathbf U \mathbf S$.

To summarize:
- To reduce the dimensionality of the data from $p$ to $k < p$, select $k$ first columns of $\mathbf U$, and $k\times k$ upper-left part of $\mathbf S$. Their product $\mathbf U_k \mathbf S_k$ is the required $n \times k$ matrix containing first $k$ PCs.
- Strictly speaking, $\mathbf U$ is of $n\times n$ size and $\mathbf V$ is of $p \times p$ size. However, if $n > p$ then the last $n-p$ columns of $\mathbf U$ will be zeros; one should therefore use an *"economy size"* (or *"thin"*) SVD that returns $\mathbf U$ of $n\times p$ size, dropping the zero columns. For large $n \gg p$ the matrix $\mathbf U$ would otherwise be unnecessarily huge. 
