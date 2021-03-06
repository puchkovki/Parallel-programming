#ifndef SKIPLIST_LIST_HPP_
#define SKIPLIST_LIST_HPP_

#include <atomic>

template< typename T>
struct ListNode {
        T key;
        std::atomic< ListNode< T >* > next_;
        std::atomic< ListNode< T >* > down_;

        // Default constructor
        ListNode(): key{T()}, next(nullptr), down(nullptr)
        {}
        // Constructor by the element
        explicit ListNode(const T& value, std::atomic< ListNode< T >* > next,
                std::atomic< ListNode< T >* > down):
                key(value), next_(next), down_(down)
        {}
    };


template< typename T>
class List {
    public:
        ListNode< T >* node;

        // Default typedefs for the list container
        typedef T value_type;
        typedef ListNode< value_type > Node;

        typedef value_type&         reference;
        typedef const value_type&   const_reference;

        typedef value_type*         pointer;
        typedef const value_type*   const_pointer;

        typedef size_t              size_type;
        typedef ptrdiff_t           difference_type;

        class Iterator {
            private:
                Node* cur;

            public:
                explicit Iterator(Node* node): cur(node)
                {}

                Iterator operator++() {
                    cur = cur->next;
                    return *this;
                }

                bool operator==(Iterator other) const {
                    return cur == other.cur;
                }
                bool operator!=(Iterator other) const {
                    return !(cur == other.cur);
                }

                T operator*() const {
                    return cur->data;
                }
        };


        Iterator begin() {
            return Iterator(head_->next);
        }
        Iterator end() {
            return Iterator(tail_);
        }
        Iterator cbegin() const {
            return Iterator(head_->next);
        }
        Iterator cend() {
            return Iterator(tail_);
        }

        // Default constructor
        explicit List(size_type n_threads = 1): head_(nullptr), tail_(nullptr),
                size_(0), n_threads_(n_threads) {
            head_ = new Node(T());
            last_ = head_;
            tail_ = new Node(T());
            last_->next = tail_;
        }

        // Fill constructor
        explicit List(size_type n, const T& val = T(), size_type n_threads = 1):
                head_(nullptr), last_(nullptr), tail_(nullptr), size_(n),
                n_threads_(1) {
            head_ = new Node(val);
            tail_ = new Node(val);
            Node* prev = nullptr;
            for (size_type i = 0; i < n; ++i) {
                Node* node_ = new Node(val);
                if (prev != nullptr) {
                    prev->next = node_;
                } else {
                    head_->next.store(node_, std::memory_order_release);
                }
                prev = node_;
            }
            last_ = prev;
            prev->next = tail_;
        }

        // Default destructor
        ~List() {
            if (head_ != nullptr) {
                Node* prev = head_;
                for (Node* _next = head_->next; _next != nullptr;
                        _next = _next->next) {
                    delete(prev);
                    prev = _next;
                }
                delete(prev);
            }
        }

        // Output all elements
        void output() {
            Node* auxiliary = head_->next.load(std::memory_order_acquire);

            while (auxiliary->next != nullptr) {
                std::cout << auxiliary->data << std::endl;
                auxiliary = auxiliary->next;
            }
            std::cout << std::endl;
        }

        // Push element the the back of the list
        void push_back(const T& data) {
            Node* new_node = new Node(data);
            // Value checks whether the CAS is successfully done
            bool is_CAS_done = false;

            while (!is_CAS_done) {
                // Store the present tail
                new_node->next = last_->next.load(std::memory_order_acquire);

                is_CAS_done = last_->next.compare_exchange_weak(tail_,
            new_node, std::memory_order_relaxed);
            }
            // Move the last element
            last_ = new_node;

            // Increment the list's size
            ++size_;
        }

        // Push element to the top of the list
        void push_front(const T& data) {
            Node* new_node = new Node(data);
            // Value checks whether the CAS is successfully done
            bool is_CAS_done = false;

            while (!is_CAS_done) {
                // Store the present first element
                new_node->next = head_->next.load(std::memory_order_acquire);

                // Auxuliary variable has another proccess intervened
                Node* head = new_node->next.load(std::memory_order_acquire);

                is_CAS_done = head_->next.compare_exchange_weak(head,
            new_node, std::memory_order_relaxed);
            }

            // Add to empty list
            if (head_ == last_) {
                last_ = new_node;
            }

            ++size_;
        }

    private:
        // Fake node refers to the first element of the list
        Node* head_;
        // Last node with the element
        Node* last_;
        // Fake node to which refers last element of the list
        Node* tail_;

        // Size of the list
        std::atomic< size_type > size_;
        // Number of the concurrent threads
        size_type n_threads_;
};


template< typename T>
#endif  // SKIPLIST_LIST_HPP_
