#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>
#include <stdexcept>


template <typename T>
class Matrix {
public:
	Matrix(size_t m = 0, size_t n = 0, const T& val = T()) :
		m(m),
		n(n),
		mat(m*n, val)
	{ }

	Matrix(const Matrix<T>& that)
	{
		*this = that;
	}

	Matrix<T>& operator=(const Matrix<T>& that)
	{
		m = that.m;
		n = that.n;
		mat = that.mat;
		
		return *this;
	}

	Matrix<T> operator+(const T& val) const
	{
		Matrix<T> m = *this;
		for (T& elem : m.mat)
			elem += val;

		return m;
	}

	T& at(size_t i, size_t j)
	{
		return mat.at(n*i + j);
	}

	const T& at(size_t i, size_t j) const
	{
		return mat.at(n*i + j);
	}

	Matrix<T>& extend(const Matrix& that)
	{
		if (width() != that.width())
			throw std::logic_error("matrix size error");

		mat.insert(mat.end(), that.mat.begin(), that.mat.end());
		m += that.m;

		return *this;
	}

	size_t height() const
	{
		return m;
	}

	size_t width() const
	{
		return n;
	}

private:
	size_t m;
	size_t n;
	std::vector<T> mat;
};

class Automaton {
public:
	Automaton(size_t states_number, size_t alphabet_size, size_t start = 0) :
		transitions(states_number, alphabet_size, NO_TRANSITION),
		terminals(states_number, false),
		start(start)
	{ }

	Automaton() {}

	Automaton(const Automaton& that)
	{
		*this = that;
	}

	Automaton& operator=(const Automaton& that)
	{
		start = that.start;
		transitions = that.transitions;
		terminals = that.terminals;

		return *this;
	}

	Automaton operator+(const Automaton& that) const
	{
		Automaton a = *this;
		a.transitions.extend(that.transitions + this->size());
		a.terminals.insert(a.terminals.end(), that.terminals.begin(), that.terminals.end());

		return a;
	}

	friend std::istream& operator>>(std::istream& input, Automaton& a)
	{
		size_t states_number, terminal_number, alphabet_size;
		size_t terminal, from, to, symb_index;
		char symbol;

		input >> states_number >> terminal_number >> alphabet_size;
		Automaton automaton(states_number, alphabet_size);

		try {
			for (size_t i = 0; i < terminal_number; i++) {
				input >> terminal;
				automaton.set_terminal(terminal);
			}
			for (size_t i = 0; i < states_number*alphabet_size; i++) {
				input >> from >> symbol >> to;
				symb_index = static_cast<size_t>(symbol - 'a');
				automaton.set_transition(from, to, symb_index);
			}
		}
		catch (const std::out_of_range& oor) {
			throw std::logic_error("input data incorrect");
		}

		a = automaton;

		return input;
	}

	bool operator==(const Automaton& that) const
	{
		if (this->size() == 0 || that.size() == 0)
			return false;

		Automaton a = *this + that;
		size_t start1 = this->start_state();
		size_t start2 = this->size() + that.start_state();

		return a.is_states_distinguishable(start1, start2);
	}

	size_t alphabet_size() const
	{
		return transitions.width();
	}

	size_t size() const
	{
		return transitions.height();
	}

	size_t start_state() const
	{
		return start;
	}

	void set_terminal(size_t vertex)
	{
		terminals.at(vertex) = true;
	}

	bool is_terminal(size_t vertex) const
	{
		return terminals.at(vertex);
	}

	size_t get_transition(size_t from, size_t symbol) const
	{
		return transitions.at(from, symbol);
	}

	void set_transition(size_t from, size_t to, size_t symbol)
	{
		if (to < size() || to == NO_TRANSITION)
			transitions.at(from, symbol) = to;
		else
			throw std::out_of_range("state index out of range");
	}

private:
	template<typename T>
	using VectorMatrix = Matrix<std::vector<T>>;

	VectorMatrix<size_t> build_inverse_edge_list() const
	{
		VectorMatrix<size_t> inv_list(transitions.height(), transitions.width());

		for (size_t i = 0; i < transitions.height(); i++) {
			for (size_t j = 0; j < transitions.width(); j++) {
				if (transitions.at(i,j) != NO_TRANSITION)
					inv_list.at(transitions.at(i,j), j).push_back(i);
			}
		}

		return inv_list;
	}

	Matrix<char> build_nonequivalence_table() const
	{
		std::queue<std::pair<size_t, size_t>> q;
		Matrix<char> marked(transitions.height(), transitions.height(), 0);

		for (size_t i = 0; i < transitions.height(); i++) {
			for (size_t j = 0; j < transitions.height(); j++) {
				if (!marked.at(i,j) && (terminals.at(i) != terminals.at(j)) ) {
					marked.at(i,j) = marked.at(j,i) = 1;
					q.push(std::make_pair(i,j));
				}
			}
		}

		size_t vertex1, vertex2;
		VectorMatrix<size_t> inv_list = build_inverse_edge_list();

		while (!q.empty()) {
			vertex1 = q.front().first;
			vertex2 = q.front().second;
			q.pop();

			for (size_t c = 0; c < alphabet_size(); c++) {
				for (size_t i : inv_list.at(vertex1, c)) {
					for (size_t j : inv_list.at(vertex2, c)) {
						if (!marked.at(i,j)) {
							marked.at(i,j) = marked.at(j,i) = 1;
							q.push(std::make_pair(i,j));
						}
					}
				}
			}
		}

		return marked;
	}

	bool is_states_distinguishable(size_t state1, size_t state2)
	{
		Matrix<char> netable = build_nonequivalence_table();
		return !netable.at(state1, state2);
	}

	Matrix<size_t> transitions;
	std::vector<bool> terminals;
	size_t start;

	static const size_t NO_TRANSITION;
};

const size_t Automaton::NO_TRANSITION = std::numeric_limits<size_t>::max();




int main()
{
	Automaton a1, a2;

	std::cin >> a1 >> a2;

	if (a1 == a2)
		std::cout << "EQUIVALENT" << std::endl;
	else
		std::cout << "NOT EQUIVALENT" << std::endl;

	return 0;
}