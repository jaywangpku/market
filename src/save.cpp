
// set取交集，并集，差集
for(int i = 0; i < this->numparts; i++){
	partitions[i]->getVertices();
	set_union(partitions[i]->vertices.begin(), partitions[i]->vertices.end(), \
			t.begin(), t.end(), inserter(t, t.begin()));
}