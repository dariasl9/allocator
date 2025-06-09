#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <cstdlib>

using ull = unsigned long long;

constexpr ull factorial(unsigned n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

template <typename T, std::size_t BlockSize>
struct MyAllocator
{
	using value_type = T;

	using pointer = T *;
	using const_pointer = const T *;
	using reference = T &;
	using const_reference = const T &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

	template <typename U>
	struct rebind
	{
		using other = MyAllocator<U, BlockSize>;
	};

    MyAllocator() : memory_block(nullptr), used(0) {
        memory_block = static_cast<pointer>(std::malloc(BlockSize * sizeof(T)));
        if (!memory_block) {
            throw std::bad_alloc();
        }
    }

    ~MyAllocator() noexcept {
        if (memory_block)
            std::free(memory_block);
    }

	template <typename U>
	MyAllocator(const MyAllocator<U, BlockSize>&) noexcept : MyAllocator() {}

    T* allocate(std::size_t n) {
        if (n == 0) return nullptr;
        if (used + n > BlockSize) {
            throw std::bad_alloc();
        }

        pointer ptr = memory_block + used;
        used += n;
        std::cout << "Allocated " << n << " elements. Total used: " << used << "/" << BlockSize << "\n";
        return ptr;
    }

    void deallocate([[maybe_unused]] T* p, [[maybe_unused]] std::size_t n) noexcept {
        std::cout << "Deallocate called (no action taken)\n";
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        std::cout << "Constructing object at " << static_cast<void*>(p) << "\n";
        new (p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) noexcept {
        std::cout << "Destroying object at " << static_cast<void*>(p) << "\n";
        p->~U();
    }

    size_type max_size() const noexcept {
        return BlockSize;
    }

private:
    pointer memory_block;
    size_type used;
};

// for C++20
template <typename T, std::size_t N>
bool operator==(const MyAllocator<T, N>&, const MyAllocator<T, N>&) noexcept {
    return true;
}

template <typename T, std::size_t N>
bool operator!=(const MyAllocator<T, N>&, const MyAllocator<T, N>&) noexcept {
    return false;
}

template <typename T>
struct ListNode {
    T data;
    ListNode* next;
    
    ListNode(const T& value) : data(value), next(nullptr) {}
};

template <typename T, typename Allocator = std::allocator<T>>
class SimpleContainer {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using node_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<ListNode<T>>;

    SimpleContainer() : head(nullptr), tail(nullptr) {}
    
    ~SimpleContainer() {
        clear();
    }

    void push_back(const T& value) {
        auto node = node_allocator.allocate(1);
        std::allocator_traits<node_allocator_type>::construct(node_allocator, node, value);
        
        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    void clear() {
        while (head) {
            auto next = head->next;
            std::allocator_traits<node_allocator_type>::destroy(node_allocator, head);
            node_allocator.deallocate(head, 1);
            head = next;
        }
        tail = nullptr;
    }

    class iterator {
    public:
        iterator(ListNode<T>* node) : current(node) {}
        
        iterator& operator++() {
            current = current->next;
            return *this;
        }
        
        bool operator!=(const iterator& other) const {
            return current != other.current;
        }
        
        T& operator*() {
            return current->data;
        }
        
    private:
        ListNode<T>* current;
    };

    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }

private:
    ListNode<T>* head;
    ListNode<T>* tail;
    node_allocator_type node_allocator;
};

int main(int, char *[])
{
    try {
        std::cout << "\n=== Testing std::map ===\n";
        std::map<ull, ull> standard_map_standard_alloc;
        [&standard_map_standard_alloc] {
            for (ull i = 0; i < 10; ++i) {
                standard_map_standard_alloc[i] = factorial(i);
                }
        }();
        std::cout << "\nPrinting std::map\n";
        for (const auto& [k, v] : standard_map_standard_alloc) {
            std::cout << k << " " << v << "; ";
        }
        std::cout <<"\n";

        std::cout << "\n=== Testing std::map with my allocator ===\n";\
        std::map<ull, ull, std::less<>, MyAllocator<std::pair<ull, ull>, 10>> standard_map_my_alloc;
        [&standard_map_my_alloc] {
            for (ull i = 0; i < 10; ++i) {
                standard_map_my_alloc[i] = factorial(i);
                }
        }();
        std::cout << "\nPrinting std::map with my allocator\n";
        for (const auto& [k, v] : standard_map_my_alloc) {
            std::cout << k << " " << v << "; ";
        }
        std::cout <<"\n";

        std::cout << "\n=== Testing my container ===\n";
        SimpleContainer<int> my_container_with_standard_alloc;
        for (int i = 0; i < 10; ++i) {
            my_container_with_standard_alloc.push_back(i);
        }
        std::cout << "\nPrinting my container with standard allocator\n";
        for (const auto& v : my_container_with_standard_alloc) {
            std::cout << v << " ";
        }
        std::cout <<"\n";

        std::cout << "\n=== Testing my container with my allocator ===\n";
        SimpleContainer<int, MyAllocator<int, 10>> my_container_with_my_alloc;
        for (int i = 0; i < 10; ++i) {
            my_container_with_my_alloc.push_back(i);
        }
        std::cout << "\nPrinting my container with my allocator\n";
        for (const auto& v : my_container_with_my_alloc) {
            std::cout << v << " ";
        }
        std::cout <<"\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

	return 0;
}