#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <string>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <sstream>
// Cambodia & Japan Est. based on Politic, Economy & Science
// Use Nonlinear Dynamical System (Continuous-Time ODE Approximation), Stochastic, Event-Driven Dynamic Shocks, Numerical Integration, etc.

// For image/video generation - using system calls to ffmpeg/gnuplot
#include <cstdlib>

// Random number generator
std::default_random_engine gen;

// Function for small Gaussian noise
double stochastic_noise(double sigma=0.01) {
    std::normal_distribution<double> dist(0.0, sigma);
    return dist(gen);
}

// Function for dynamic shocks: political crisis, financial crisis, pandemic
double dynamic_shock(double time, double probability, double magnitude) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(gen) < probability) {
        std::cout << "Shock at year " << time << " with magnitude " << magnitude << "\n";
        return magnitude * ((dist(gen) < 0.5) ? -1.0 : 1.0);
    }
    return 0.0;
}

// Structure to define a country
struct Country {
    std::string name;
    double a, b, c, d, e, f; // model parameters
    double P0, S0, E0;        // initial values
    double noiseP, noiseS, noiseE; // noise levels
};

// Structure to store simulation data for visualization
struct SimulationData {
    std::vector<double> years;
    std::vector<double> P_values;
    std::vector<double> S_values;
    std::vector<double> E_values;
    std::vector<double> shock_marks; // Track when shocks occur
};

// Function to create directory for outputs
void create_output_directories() {
    system("mkdir -p frames tracking videos");
}

// Function to generate GNUplot script for a single frame
void generate_frame_script(const SimulationData& data, int frame_num, double current_year, 
                          const std::string& country_name, const std::string& output_dir) {
    std::ofstream script("temp_plot.gp");
    
    script << "set terminal pngcairo size 1200,800 enhanced font 'Arial,12'\n";
    script << "set output '" << output_dir << "/frame_" << std::setfill('0') << std::setw(5) << frame_num << ".png'\n";
    script << "set title '" << country_name << " Development Simulation - Year " 
           << std::fixed << std::setprecision(1) << current_year << "'\n";
    script << "set xlabel 'Year'\n";
    script << "set ylabel 'Index (0-1)'\n";
    script << "set xrange [1990:2100]\n";
    script << "set yrange [0:1]\n";
    script << "set grid\n";
    script << "set key outside right\n";
    
    // Plot historical data and current point
    script << "plot '-' using 1:2 with lines title 'Political Stability (P)' lw 2 lc rgb '#4CAF50', \\\n";
    script << "     '-' using 1:3 with lines title 'Social Development (S)' lw 2 lc rgb '#2196F3', \\\n";
    script << "     '-' using 1:4 with lines title 'Economic Development (E)' lw 2 lc rgb '#FF9800', \\\n";
    script << "     '-' using 1:2 with points title 'Current' pt 7 ps 2 lc rgb '#F44336'\n";
    
    // Write data up to current year
    for (size_t i = 0; i < data.years.size() && data.years[i] <= current_year; i++) {
        script << data.years[i] << " " << data.P_values[i] << " " 
               << data.S_values[i] << " " << data.E_values[i] << "\n";
    }
    script << "e\n";
    
    // Repeat for S and E (gnuplot requires separate data blocks)
    for (size_t i = 0; i < data.years.size() && data.years[i] <= current_year; i++) {
        script << data.years[i] << " " << data.P_values[i] << " " 
               << data.S_values[i] << " " << data.E_values[i] << "\n";
    }
    script << "e\n";
    
    for (size_t i = 0; i < data.years.size() && data.years[i] <= current_year; i++) {
        script << data.years[i] << " " << data.P_values[i] << " " 
               << data.S_values[i] << " " << data.E_values[i] << "\n";
    }
    script << "e\n";
    
    // Current point
    if (!data.years.empty()) {
        size_t last_idx = 0;
        for (size_t i = 0; i < data.years.size() && data.years[i] <= current_year; i++) {
            last_idx = i;
        }
        script << data.years[last_idx] << " " << data.P_values[last_idx] << " "
               << data.S_values[last_idx] << " " << data.E_values[last_idx] << "\n";
    }
    script << "e\n";
    
    script.close();
    
    // Execute gnuplot
    std::string cmd = "gnuplot temp_plot.gp 2>/dev/null";
    system(cmd.c_str());
}

// Function to generate final tracking image
void generate_tracking_image(const SimulationData& data, const std::string& country_name) {
    std::ofstream script("tracking_plot.gp");
    
    script << "set terminal jpeg size 1600,1000 enhanced\n";
    script << "set output 'tracking/" << country_name << "_tracking_1990_2100.jpg'\n";
    script << "set title '" << country_name << " Development Tracking (1990-2100)' font 'Arial,16'\n";
    script << "set xlabel 'Year'\n";
    script << "set ylabel 'Index (0-1)'\n";
    script << "set xrange [1990:2100]\n";
    script << "set yrange [0:1]\n";
    script << "set grid\n";
    script << "set key outside right\n";
    
    // Multiplot layout for main trajectory and phase space
    script << "set multiplot layout 2,2 title '" << country_name << " Development Analysis'\n";
    
    // Main time series
    script << "set size 0.7,0.7\n";
    script << "set origin 0,0.3\n";
    script << "plot '-' using 1:2 with lines title 'Political (P)' lw 2 lc rgb '#4CAF50', \\\n";
    script << "     '-' using 1:3 with lines title 'Social (S)' lw 2 lc rgb '#2196F3', \\\n";
    script << "     '-' using 1:4 with lines title 'Economic (E)' lw 2 lc rgb '#FF9800'\n";
    
    // Write all data
    for (size_t i = 0; i < data.years.size(); i++) {
        script << data.years[i] << " " << data.P_values[i] << " " 
               << data.S_values[i] << " " << data.E_values[i] << "\n";
    }
    script << "e\n";
    for (size_t i = 0; i < data.years.size(); i++) {
        script << data.years[i] << " " << data.P_values[i] << " " 
               << data.S_values[i] << " " << data.E_values[i] << "\n";
    }
    script << "e\n";
    for (size_t i = 0; i < data.years.size(); i++) {
        script << data.years[i] << " " << data.P_values[i] << " " 
               << data.S_values[i] << " " << data.E_values[i] << "\n";
    }
    script << "e\n";
    
    // Phase space: P vs E
    script << "set size 0.3,0.3\n";
    script << "set origin 0.7,0.7\n";
    script << "set title 'Political-Economic Phase Space'\n";
    script << "set xlabel 'Economic (E)'\n";
    script << "set ylabel 'Political (P)'\n";
    script << "plot '-' using 4:2 with lines title '' lw 2 lc rgb '#9C27B0'\n";
    for (size_t i = 0; i < data.years.size(); i++) {
        script << data.E_values[i] << " " << data.P_values[i] << "\n";
    }
    script << "e\n";
    
    // Phase space: S vs E
    script << "set origin 0.7,0.4\n";
    script << "set title 'Social-Economic Phase Space'\n";
    script << "set xlabel 'Economic (E)'\n";
    script << "set ylabel 'Social (S)'\n";
    script << "plot '-' using 4:3 with lines title '' lw 2 lc rgb '#FF5722'\n";
    for (size_t i = 0; i < data.years.size(); i++) {
        script << data.E_values[i] << " " << data.S_values[i] << "\n";
    }
    script << "e\n";
    
    // Phase space: P vs S
    script << "set origin 0.7,0.1\n";
    script << "set title 'Political-Social Phase Space'\n";
    script << "set xlabel 'Social (S)'\n";
    script << "set ylabel 'Political (P)'\n";
    script << "plot '-' using 3:2 with lines title '' lw 2 lc rgb '#00BCD4'\n";
    for (size_t i = 0; i < data.years.size(); i++) {
        script << data.S_values[i] << " " << data.P_values[i] << "\n";
    }
    script << "e\n";
    
    script << "unset multiplot\n";
    script.close();
    
    system("gnuplot tracking_plot.gp 2>/dev/null");
    std::cout << "Generated tracking image: tracking/" << country_name << "_tracking_1990_2100.jpg\n";
}

// Function to create video from frames
void create_video(const std::string& country_name, int total_frames) {
    std::cout << "Creating MP4 video for " << country_name << "...\n";
    
    // Create video using ffmpeg
    std::string ffmpeg_cmd = "ffmpeg -y -framerate 30 -i frames/" + country_name + "_frame_%05d.png "
                             "-c:v libx264 -pix_fmt yuv420p -preset medium -crf 23 "
                             "videos/" + country_name + "_simulation_1990_2100.mp4 2>/dev/null";
    
    int result = system(ffmpeg_cmd.c_str());
    
    if (result == 0) {
        std::cout << "Video created: videos/" << country_name << "_simulation_1990_2100.mp4\n";
    } else {
        std::cout << "Error creating video. Make sure ffmpeg is installed.\n";
    }
}

// Function to generate summary statistics
void generate_summary_stats(const SimulationData& data, const std::string& country_name) {
    std::ofstream stats("tracking/" + country_name + "_statistics.txt");
    
    stats << "=== " << country_name << " Development Simulation Statistics ===\n";
    stats << "Period: 1990 - 2100\n\n";
    
    // Initial values
    stats << "INITIAL VALUES (1990):\n";
    stats << "  Political Stability (P): " << std::fixed << std::setprecision(3) << data.P_values[0] << "\n";
    stats << "  Social Development (S): " << data.S_values[0] << "\n";
    stats << "  Economic Development (E): " << data.E_values[0] << "\n\n";
    
    // Final values
    stats << "FINAL VALUES (2100):\n";
    stats << "  Political Stability (P): " << data.P_values.back() << "\n";
    stats << "  Social Development (S): " << data.S_values.back() << "\n";
    stats << "  Economic Development (E): " << data.E_values.back() << "\n\n";
    
    // Maximum values
    stats << "MAXIMUM VALUES:\n";
    stats << "  Political Stability (P): " << *std::max_element(data.P_values.begin(), data.P_values.end()) << "\n";
    stats << "  Social Development (S): " << *std::max_element(data.S_values.begin(), data.S_values.end()) << "\n";
    stats << "  Economic Development (E): " << *std::max_element(data.E_values.begin(), data.E_values.end()) << "\n\n";
    
    // Minimum values
    stats << "MINIMUM VALUES:\n";
    stats << "  Political Stability (P): " << *std::min_element(data.P_values.begin(), data.P_values.end()) << "\n";
    stats << "  Social Development (S): " << *std::min_element(data.S_values.begin(), data.S_values.end()) << "\n";
    stats << "  Economic Development (E): " << *std::min_element(data.E_values.begin(), data.E_values.end()) << "\n\n";
    
    // Growth rates
    double total_years = data.years.back() - data.years.front();
    double P_growth = (data.P_values.back() - data.P_values.front()) / total_years * 100;
    double S_growth = (data.S_values.back() - data.S_values.front()) / total_years * 100;
    double E_growth = (data.E_values.back() - data.E_values.front()) / total_years * 100;
    
    stats << "AVERAGE ANNUAL GROWTH RATE (%):\n";
    stats << "  Political Stability (P): " << std::setprecision(2) << P_growth << "%\n";
    stats << "  Social Development (S): " << S_growth << "%\n";
    stats << "  Economic Development (E): " << E_growth << "%\n\n";
    
    // Shock summary
    stats << "SHOCK EVENTS:\n";
    stats << "  Total shocks detected: " << data.shock_marks.size() << "\n";
    for (size_t i = 0; i < data.shock_marks.size(); i++) {
        stats << "  Shock " << i+1 << " at year " << std::setprecision(1) << data.shock_marks[i] << "\n";
    }
    
    stats.close();
    std::cout << "Generated statistics: tracking/" << country_name << "_statistics.txt\n";
}

int main() {
    // Create output directories
    create_output_directories();
    
    // Define Cambodia (developing) and Japan (developed)
    Country countries[2] = {
        {"Cambodia", 0.3, 0.5, 0.2, 0.4, 0.3, 0.35, 0.5, 0.2, 0.4, 0.005, 0.005, 0.01},
        {"Japan",    0.5, 0.1, 0.4, 0.1, 0.6, 0.05, 0.9, 0.8, 0.85, 0.002, 0.002, 0.003}
    };

    double start_year = 1990;
    double end_year = 2100;
    double dt = 0.1; // smaller dt for smooth simulation (0.1 year per step)
    int steps = static_cast<int>((end_year - start_year) / dt);
    
    // Frame rate for video (every 5 steps = 0.5 year intervals for smoother video)
    int frame_interval = 5;
    int frame_count = 0;

    for (auto &country : countries) {
        std::cout << "\n========================================\n";
        std::cout << "Simulating " << country.name << "\n";
        std::cout << "========================================\n";
        
        // CSV file for raw data
        std::ofstream file(country.name + "_1990_2100.csv");
        file << "year,P,S,E\n";

        // Store simulation data for visualization
        SimulationData sim_data;
        sim_data.years.reserve(steps + 1);
        sim_data.P_values.reserve(steps + 1);
        sim_data.S_values.reserve(steps + 1);
        sim_data.E_values.reserve(steps + 1);

        double P = country.P0;
        double S = country.S0;
        double E = country.E0;

        for (int i = 0; i <= steps; i++) {
            double year = start_year + i * dt;

            // Base nonlinear logistic growth
            double dP = country.a * E / (1 + E) * (1 - P) - country.b * P + stochastic_noise(country.noiseP);
            double dS = country.c * P * (1 - S) * (1 - S) - country.d * S + stochastic_noise(country.noiseS);
            double dE = country.e * S * S / (1 + S * S) * (1 - E) - country.f * E + stochastic_noise(country.noiseE);

            // Add dynamic shocks (probabilities are per dt)
            double shock_P = dynamic_shock(year, 0.001, 0.2); // Political crisis
            double shock_E = dynamic_shock(year, 0.0005, 0.3); // Financial crisis
            double shock_S = dynamic_shock(year, 0.0008, 0.15); // Pandemic
            
            dP += shock_P;
            dE += shock_E;
            dS += shock_S;
            
            // Record shocks
            if (shock_P != 0 || shock_E != 0 || shock_S != 0) {
                sim_data.shock_marks.push_back(year);
            }

            // Update variables
            P += dP * dt;
            S += dS * dt;
            E += dE * dt;

            // Clamp between 0 and 1
            P = std::clamp(P, 0.0, 1.0);
            S = std::clamp(S, 0.0, 1.0);
            E = std::clamp(E, 0.0, 1.0);

            // Store data
            sim_data.years.push_back(year);
            sim_data.P_values.push_back(P);
            sim_data.S_values.push_back(S);
            sim_data.E_values.push_back(E);

            // Write to CSV
            file << year << "," << P << "," << S << "," << E << "\n";
            
            // Generate frame for video at intervals
            if (i % frame_interval == 0) {
                generate_frame_script(sim_data, frame_count, year, country.name, "frames");
                frame_count++;
                
                // Progress indicator
                if (i % (steps/10) == 0) {
                    std::cout << "Progress: " << (i * 100 / steps) << "%\r" << std::flush;
                }
            }
        }

        file.close();
        std::cout << "\nCSV data saved: " << country.name << "_1990_2100.csv\n";
        
        // Generate final tracking image
        generate_tracking_image(sim_data, country.name);
        
        // Generate statistics
        generate_summary_stats(sim_data, country.name);
        
        // Create video from frames
        create_video(country.name, frame_count);
        
        // Reset frame counter for next country
        frame_count = 0;
    }

    // Clean up temporary files
    system("rm -f temp_plot.gp tracking_plot.gp");
    
    std::cout << "\n========================================\n";
    std::cout << "Simulation complete!\n";
    std::cout << "Output files:\n";
    std::cout << "  - CSV data: *_1990_2100.csv\n";
    std::cout << "  - Tracking images: tracking/*.jpg\n";
    std::cout << "  - Videos: videos/*.mp4\n";
    std::cout << "  - Statistics: tracking/*_statistics.txt\n";
    std::cout << "========================================\n";

    return 0;
}