# script for saturating counter calculations
if __name__ == "__main__":
    var_array = ["pc_0", "pc_1", "pc_2", "pc_3", "tag_1", "tag_2"]
    threshold = [31,-32]
    for var in var_array:
        print("if (%s_feature[%s_hash] < %d) {" % (var, var, threshold[0]))
        print("\t%s_feature[%s_hash]++;" % (var, var))
        print("}")
    for var in var_array:
        print("if (%s_feature[%s_hash] > %d) {" % (var, var, threshold[1]))
        print("\t%s_feature[%s_hash]--;" % (var, var))
        print("}")