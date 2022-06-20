#include <iostream>
#include <cstdint>

struct bmat new_bmatrix(int rows, int cols) {
    int number_of_bits = rows * cols;
    int number_of_chunks = number_of_bits / bit_density;
    
    // Round up if we need to fill a partial chunk to fit all of the data
    if (number_of_chunks * 64 < number_of_bits) {
        number_of_chunks++;
    }
    //std::cout << "Making a total of " << number_of_chunks << " chunks" << "\n";

    // Watch out for memory leaks!!
    uint64_t* bmatrix_data = new uint64_t[number_of_chunks];
    for (int idx = 0; idx < number_of_chunks; idx++) {
        *(bmatrix_data + idx) = zero;
    }

    bmat bmatrix = { bmatrix_data, rows, cols, number_of_chunks };
    return bmatrix;
}

// The three index styles:
// 
// Macroscopic Index a.k.a. 'macro_index' 
// (row, col) ... (0 <= row < total number of rows, 0 <= col < total number of columns)
// - The Macroscopic Index is the 'standard' way of indexing matrices.  It is dependent on the dimensions of the matrix, so conversion to it requires you to check the dimensions.
// - Both of the other index types (BI, DI) are not dependent on the matrix dimensions, so they can be converted easily without referring to the actual matrix data.
// - MI should be the only index used outside of internal matrix manipulations.
// 
// <->
// 
//      Bit Index a.k.a. 'bit_index' 
//      (bit_number a.k.a. 'idx') ... (0 < bit_number < (total number of rows) * (total number of columns))
//      - Represents the matrix as if it were flattened out into a single long vector, containing all of the elements
//      <->
// 
//           Data Index a.k.a. 'data_index' 
//           (data chunk number a.k.a. 'c_idx', bit number within data chunk 'd_idx')
//           (0 < c_idx < total number of chunks, 0 <= d_idx < bit_density)
//           - Represents the data in chunks of bit_density bits.  Each row is bit_density long, and stacked to make enough entries.  The last row may not be fully utilized.
//

void print_BI(bit_index to_print);
void print_BI(bit_index to_print) {
    //std::cout << "BI: " << (int)to_print << "\n";
}

void print_MI(macro_index to_print);
void print_MI(macro_index to_print) {
    //std::cout << "MI: " << (int)to_print.row << ", " << (int)to_print.col << "\n";
}

void print_DI(data_index to_print);
void print_DI(data_index to_print) {
    //std::cout << "DI: " << (int)to_print.c_idx << ", " << (int)to_print.d_idx << "\n";
}

// Below is index conversion code

// Macroscopic Index <-> Bit Index
bit_index get_BI_from_MI(macro_index for_conversion, bmat matrix_for_indexing);
bit_index get_BI_from_MI(macro_index for_conversion, bmat matrix_for_indexing) {
    bit_index conversion_result = (bit_index)(for_conversion.row * matrix_for_indexing.cols + for_conversion.col);
    return (conversion_result);
}

struct macro_index get_MI_from_BI(bit_index for_conversion, bmat matrix_for_indexing);
struct macro_index get_MI_from_BI(bit_index for_conversion, bmat matrix_for_indexing) {
    macro_index conversion_result = {(int)for_conversion / matrix_for_indexing.cols, (int)for_conversion % matrix_for_indexing.cols };
    return (conversion_result);
}

// Bit Index <-> Data Index
struct data_index get_DI_from_BI(bit_index idx);
struct data_index get_DI_from_BI(bit_index idx) {
    data_index conversion_result = {(int)(idx / bit_density), (int)(idx % bit_density) };
    return(conversion_result);
}

bit_index get_BI_from_DI(data_index to_convert);
bit_index get_BI_from_DI(data_index to_convert) {
    return((bit_index)(to_convert.c_idx * bit_density + to_convert.d_idx));
}

// Macroscopic Index <-> Data Index
struct data_index get_DI_from_MI(macro_index for_conversion, bmat matrix_for_indexing);
struct data_index get_DI_from_MI(macro_index for_conversion, bmat matrix_for_indexing) {
    return(get_DI_from_BI(get_BI_from_MI(for_conversion, matrix_for_indexing)));
}

struct macro_index get_MI_from_DI(data_index to_convert, bmat matrix_of_interest);
struct macro_index get_MI_from_DI(data_index to_convert, bmat matrix_of_interest) {
    return(get_MI_from_BI(get_BI_from_DI(to_convert), matrix_of_interest));
}

void test_idx_conversion(bmat test_bmat);
void test_idx_conversion(bmat test_bmat) {
    //std::cout << "Testing index conversion..." << "\n";
    macro_index test_MI = { 11, 11 };
    print_MI(test_MI);
    bit_index test_BI = get_BI_from_MI(test_MI, test_bmat);
    print_BI(test_BI);
    data_index test_DI = get_DI_from_BI(test_BI);
    print_DI(test_DI);
    test_MI = get_MI_from_DI(test_DI, test_bmat);
    print_MI(test_MI);
    //std::cout << "\n";

    macro_index test_result;
    for (int i = 0; i < test_bmat.rows; i++) {
        for (int j = 0; j < test_bmat.cols; j++) {
            test_MI = { i, j };
            print_MI(test_MI);
            test_BI = get_BI_from_MI(test_MI, test_bmat);
            test_DI = get_DI_from_BI(test_BI);
            test_result = get_MI_from_DI(test_DI, test_bmat);
            if (test_MI.row != test_result.row || test_MI.col != test_result.col) {
                //std::cout << "TEST FAILED" << "\n";
                break;
            }
        }
    }
}

// Storing and retrieving data is done using the MI
//void print_binary(uint64_t v);
//void print_binary(uint64_t v) {
//    for (int i = bit_density - 1; i >= 0; i--)
//        std::cout << ((v >> i) & one);
//    std::cout << "\n";
//}

// This function is for setting a bit in an unsigned integer
uint64_t set_bit(uint64_t input, uint64_t bit_idx, uint64_t data) {
//    if (bit_idx >= (uint64_t)bit_density) {
//        throw std::invalid_argument("Invalid bit index.");
//    }
    uint64_t output = (input & ~(one << bit_idx)) | (data << bit_idx);
    return(output);
}

// Toggling
uint64_t toggle(uint64_t input, uint64_t bit_idx) {
//    if (bit_idx >= (uint64_t)bit_density) {
//        throw std::invalid_argument("Invalid bit index.");
//    }
    uint64_t output = input ^ (one << bit_idx);
    return(output);
}

uint64_t read_bit(uint64_t input, uint64_t bit_idx) {
    return((input >> bit_idx) & one);
}

//void test_bit_setting();
//void test_bit_setting() {
//    uint64_t v;
//    for (uint64_t j = 0; j < bit_density; j++) {
//        v = set_bit(zero, j, one);
//        print_binary(set_bit(zero, j, one));
//    }
//}

// Store bit in bmat
void store_bit(bmat& to_manipulate, macro_index entry_idx, bool value);
void store_bit(bmat& to_manipulate, macro_index entry_idx, bool value) {
    data_index spot = get_DI_from_MI(entry_idx, to_manipulate);
    *(to_manipulate.bmatrix_data + spot.c_idx) = set_bit(*(to_manipulate.bmatrix_data + spot.c_idx), spot.d_idx, (uint64_t)value);
}

// Toggle bit
void toggle_bit(bmat& to_manipulate, macro_index entry_idx, bool value);
void toggle_bit(bmat& to_manipulate, macro_index entry_idx) {
    data_index spot = get_DI_from_MI(entry_idx, to_manipulate);
    *(to_manipulate.bmatrix_data + spot.c_idx) = toggle(*(to_manipulate.bmatrix_data + spot.c_idx), spot.d_idx);
}

// Read bit from bmat
uint64_t get_bit(bmat to_get_from, macro_index where_to_get);
uint64_t get_bit(bmat to_get_from, macro_index where_to_get) {
    data_index translation = get_DI_from_MI(where_to_get, to_get_from);
    return(read_bit(*(to_get_from.bmatrix_data + translation.c_idx), translation.d_idx));
}

uint64_t get_bit(uint64_t input, uint64_t bit_idx, uint64_t data);
uint64_t get_bit(uint64_t input, uint64_t bit_idx, uint64_t data) {
//    if (bit_idx >= (uint64_t)bit_density) {
//        throw //std::invalid_argument("Invalid bit index.");
//    }
    uint64_t output = (input & ~(one << bit_idx)) | (data << bit_idx);
    return(output);
}

void print_bmat(bmat to_print);
void print_bmat(bmat to_print) {
    macro_index current_bit = { 0, 0 };
    for (int row = 0; row < to_print.rows; row++) {
        for (int col = 0; col < to_print.cols; col++) {
            current_bit = { row, col };
            //std::cout << get_bit(to_print, current_bit);
        }
        //std::cout << "\n";
    }
}

bmat identity(int n) {

    bmat identity = new_bmatrix(n, n);
    macro_index current_bit = { 0,0 };
    for (int idx = 0; idx < n; idx++) {
        current_bit = { idx,idx };
        store_bit(identity, current_bit, one);
     }
    return identity;
}
