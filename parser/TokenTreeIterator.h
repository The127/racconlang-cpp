#pragma once


class TokenTreeIterator {
    using InnerIterator = std::vector<TokenTreeNode>::const_iterator;
private:
    InnerIterator begin;
    InnerIterator current;
    InnerIterator end;

public:
    explicit TokenTreeIterator(const std::vector<TokenTreeNode>& vec) {
        begin = current = vec.begin();
        end = vec.end();
    }

    TokenTreeIterator(InnerIterator start, InnerIterator end) {
        begin = current = start;
        this->end = end;
    }

    [[nodiscard]] bool isEnd() const {
        return current >= end;
    }

    explicit operator bool() const {
        return current < end;
    }

    const TokenTreeNode& operator*() const {
        COMPILER_ASSERT(current < end && current >= begin, "attempt to access invalid iterator");
        return current.operator*();
    }

    const TokenTreeNode* operator->() const {
        COMPILER_ASSERT(current < end && current >= begin, "attempt to access invalid iterator");
        return current.operator->();
    }

    TokenTreeIterator operator+(InnerIterator::difference_type n) const {
        return TokenTreeIterator(current.operator+(n), end);
    }

    TokenTreeIterator& operator+=(InnerIterator::difference_type n) {
        current.operator+=(n);
        return *this;
    }

    TokenTreeIterator operator-(InnerIterator::difference_type n) const {
        return TokenTreeIterator(current.operator-(n), end);
    }

    TokenTreeIterator& operator-=(InnerIterator::difference_type n) {
        current.operator-=(n);
        return *this;
    }

    bool operator==(const TokenTreeIterator& other) const {
        return current == other.current || isEnd() && other.isEnd();
    }

    bool operator!=(const TokenTreeIterator& other) const {
        if (isEnd()) {
            return !other.isEnd();
        }
        return current != other.current;
    }
};