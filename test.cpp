class T
{
private:
    int x_;
public:
    T();
    T(int x) : x_(x) {} 
    T operator=(int);
};

int main()
{
    T t = 1;
}
