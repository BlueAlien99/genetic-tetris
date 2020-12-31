#include "AI/evolutionary_strategy.hpp"

#include "rapidjson/document.h"
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <fstream>

void EvolutionaryStrategy::operator()() {
    finish_ = false;
    drop_mutex_.unlock();
    evolution_thread = std::thread([this]() { evolve(); });
    controlLoop();
}

void EvolutionaryStrategy::operator()(const std::string& input_json,
                                      const std::string& output_json) {
    finish_ = false;
    drop_mutex_.unlock();
    evolution_thread =
        std::thread([this, input_json, output_json]() { evolve(input_json, output_json); });
    controlLoop();
}

void EvolutionaryStrategy::controlLoop() {
    drop_mutex_.lock();
    while (!finish_) {
        drop_mutex_.lock();
        Genome best_cpy = best;
        Move move = generateBestMove(best_cpy, tetris_);
        move.apply(tetris_);
        if (tetris_.isFinished()) {
            finish();
        }
    }
}

void EvolutionaryStrategy::finish() {
    if (finish_) return;
    finish_ = true;
    drop();
    evolution_thread.join();
}

void EvolutionaryStrategy::saveToJSON(const std::string& file, std::vector<Genome>& genomes) {
    using namespace rapidjson;
    std::cout << "Saving genomes to JSON: " << file << std::endl;
    Document d;
    Value genomes_json(kArrayType);
    for (const auto& g : genomes) {
        Value g_json(kObjectType);
        Value id(g.id);
        Value rows_cleared(g.rows_cleared);
        Value max_height(g.max_height);
        Value cumulative_height(g.cumulative_height);
        Value relative_height(g.relative_height);
        Value holes(g.holes);
        Value roughness(g.roughness);
        g_json.AddMember("id", id, d.GetAllocator());
        g_json.AddMember("rows_cleared", rows_cleared, d.GetAllocator());
        g_json.AddMember("max_height", max_height, d.GetAllocator());
        g_json.AddMember("cumulative_height", cumulative_height, d.GetAllocator());
        g_json.AddMember("relative_height", relative_height, d.GetAllocator());
        g_json.AddMember("holes", holes, d.GetAllocator());
        g_json.AddMember("roughness", roughness, d.GetAllocator());
        genomes_json.PushBack(g_json, d.GetAllocator());
    }
    std::ofstream ofs(file);
    OStreamWrapper osw(ofs);
    Writer<OStreamWrapper> writer(osw);
    genomes_json.Accept(writer);
}

std::vector<Genome> EvolutionaryStrategy::loadFromJSON(const std::string& file) {
    using namespace rapidjson;
    std::cout << "Loading genomes from JSON: " << file << std::endl;
    std::vector<Genome> pop;
    std::ifstream ifs(file);
    IStreamWrapper isw(ifs);
    Document d;
    d.ParseStream(isw);
    assert(d.IsArray());
    assert(d.Size() == POP_SIZE);
    for (Value::ConstValueIterator itr = d.Begin(); itr != d.End(); ++itr) {
        Genome g;
        auto g_json = itr->GetObject();
        g.id = g_json["id"].GetDouble();
        g.rows_cleared = g_json["rows_cleared"].GetDouble();
        g.max_height = g_json["max_height"].GetDouble();
        g.cumulative_height = g_json["cumulative_height"].GetDouble();
        g.relative_height = g_json["relative_height"].GetDouble();
        g.holes = g_json["holes"].GetDouble();
        g.roughness = g_json["roughness"].GetDouble();
        pop.push_back(g);
    }
    assert(pop.size() == POP_SIZE);
    return pop;
}

void EvolutionaryStrategy::evolve() {
    t = 0;
    auto pop = initialPop();
    while (!finish_) {
        pop = next_generation(pop);
    }
}

void EvolutionaryStrategy::evolve(const std::string& input_json, const std::string& output_json) {
    t = 0;
    std::vector<Genome> pop;
    if (input_json != "") {
        pop = loadFromJSON(input_json);
        evaluation(pop);
    }
    else
        pop = initialPop();
    while (!finish_) {
        pop = next_generation(pop);
    }
    if (output_json != "") saveToJSON(output_json, pop);
}

std::vector<Genome> EvolutionaryStrategy::next_generation(std::vector<Genome>& pop) {
    auto selected = selection(pop);
    auto next_pop = crossoverAndMutation(selected);
    evaluation(next_pop);
    displayState();
    t++;
    return next_pop;
}

std::vector<Genome> EvolutionaryStrategy::initialPop() {
    std::vector<Genome> initial_pop(POP_SIZE);
    evaluation(initial_pop);
    return initial_pop;
}

std::vector<Genome> EvolutionaryStrategy::selection(std::vector<Genome>& pop) {
    std::vector<Genome> selected;
    selected.reserve(POP_SIZE);
    selected.push_back(best);
    while (selected.size() < SELECTED_TO_CROSS_AND_MUTATE) {
        selected.push_back(rouletteSelection(pop));
    }
    return selected;
}

std::vector<Genome> EvolutionaryStrategy::crossoverAndMutation(const std::vector<Genome> selected) {
    std::vector<Genome> next_pop(selected);
    while (next_pop.size() < POP_SIZE - 1) {
        float p = random_0_1();
        if (p < PROB_CROSSOVER) {
            auto father = rouletteSelection(selected);
            auto mother = rouletteSelection(selected);
            Genome child;
            child.max_height = (father.max_height + mother.max_height) / 2.0f;
            next_pop.push_back(child);
        } else {
            float dx = std::clamp(random_0_1() - 0.5f, -MUTATION_STRENGTH, MUTATION_STRENGTH);
            auto mutant = rouletteSelection(selected);
            mutant.max_height += dx;
        }
    }
    next_pop.push_back(best);
    return next_pop;
}

void EvolutionaryStrategy::evaluation(std::vector<Genome>& next_pop) {
    score_sum = 0.0f;
    for (auto& c : next_pop) {
        //Tetris tmp(tetris_);
        Tetris tmp;
        Move best_move;
        for (int i = 0; i < MOVES_TO_SIMULATE; i++) {
            best_move = generateBestMove(c, tmp);
            best_move.apply(tmp);
        }
        c.score = 100 - best_move.getMaxHeight() - best_move.getHoles();
        score_sum += c.score;
    }
    float prev_ps = 0.0f;
    for (auto& c : next_pop) {
        c.ps = prev_ps + c.score / score_sum;
        prev_ps = c.ps;
    }

    best = *std::max_element(next_pop.begin(), next_pop.end(), [](const Genome& a, const Genome& b) {
        return a.score < b.score;
    });
}

Move EvolutionaryStrategy::generateBestMove(const Genome& genome, Tetris& tetris) {
    Move best_move;
    float best_fitness = 0.0f;
    int i = 0;
    for (int mx = Move::MIN_MOVE; mx <= Move::MAX_MOVE; mx++) {
        for (int rot = Move::MIN_ROT; rot <= Move::MAX_ROT; rot++) {
            Tetris tmp(tetris);
            Move move(mx, rot);
            move.apply(tmp);
            float fitness =
                genome.max_height * move.getMaxHeight() + genome.holes * move.getHoles();
            fitness = std::clamp(fitness, -20.0f, 20.0f) + 20.0f;
            if (fitness > best_fitness) {
                best_fitness = fitness;
                best_move = move;
            }
            i++;
        }
    }
    return best_move;
}

void EvolutionaryStrategy::displayState() {
    mean_fitness_ = score_sum / POP_SIZE;
    std::cout << "Generation " << t << ": " << std::endl;
    std::cout << "\tmean fitness: " << mean_fitness_ << std::endl;
    printf("\tbest: (score=%f max_height=%f holes=%f)\n", best.score, best.max_height, best.holes);
}