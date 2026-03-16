template<typename T>
void print(T t) {

}

extern template void print<int>(int);
int main() {
  return 0;
}
